// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <functional>

#include "rtsp/zc_rtsp_smgr_handle.h"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"
#include "zc_msg_codec.h"
#include "zc_msg_rtsp.h"
#include "zc_msg_sys.h"
#include "zc_proc.h"
#include "zc_type.h"

#include "ZcModBase.hpp"
#include "ZcModSysBase.hpp"
#include "ZcType.hpp"

namespace zc {
CModSysBase::CModSysBase() : CModBase(ZC_MODID_SYS_E), m_init(false) {}

CModSysBase::~CModSysBase() {}

bool CModSysBase::init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    auto svrreq = std::bind(&CModSysBase::reqSvrRecvReqProc, this, std::placeholders::_1, std::placeholders::_2,
                            std::placeholders::_3, std::placeholders::_4);

    if (!initReqSvr(svrreq)) {
        LOG_ERROR("InitComm error modid:%d, url:%s", m_modid, m_url);
        goto _err;
    }

    m_init = true;
    LOG_TRACE("init ok modid:%d, url:%s", m_modid, m_url);
    return true;

_err:
    LOG_ERROR("Init error");
    return false;
}

bool CModSysBase::unInit() {
    if (!m_init) {
        return true;
    }

    unInitReqSvr();
    m_init = false;
    LOG_TRACE("unInit ok");
    return false;
}

// sys mod
zc_msg_errcode_e CModSysBase::checkReqSvrRecvReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    if (unlikely(_checklicense() < 0)) {
        LOG_TRACE("mod license error pid:%d,modid:%u", req->pid, req->modid);
        return ZC_MSG_ERR_LICENSE_E;
    }

    int ret = ZC_MSG_ERR_E;
    static ZC_U32 registerkey = (ZC_MID_SYS_MAN_E << 16) | ZC_MSID_SYS_MAN_REGISTER_E;
    // TODO(zhoucc) find msg procss mod

    ZC_U32 key = (req->id << 16) | req->sid;
    if (likely(key != registerkey)) {
        if (updateStatus(req)) {
            ret = MsgReqProc(req, iqsize, rep, opsize);
        } else {
            ret = ZC_MSG_ERR_UNREGISTER_E;
        }
    } else {
        zc_mod_reg_t *reqreg = reinterpret_cast<zc_mod_reg_t *>(req->data);
        ret = MsgReqProc(req, iqsize, rep, opsize);
        if (ret >= 0) {
            if (reqreg->regcmd == ZC_SYS_REGISTER_E) {
                registerInsert(req);
            } else if (reqreg->regcmd == ZC_SYS_UNREGISTER_E) {
                // handle error or unregister
                unregisterRemove(req);
            }
        }
    }

    return (zc_msg_errcode_e)ret;
}

// sys reqproc
ZC_S32 CModSysBase::reqSvrRecvReqProc(char *req, int iqsize, char *rep, int *opsize) {
    int ret = 0;
    zc_msg_t *reqmsg = reinterpret_cast<zc_msg_t *>(req);
    zc_msg_t *repmsg = reinterpret_cast<zc_msg_t *>(rep);

    if (reqmsg->modid != ZC_MODID_SYS_E) {
        // msg not from sysmod, check register and keeplive status
        ret = checkReqSvrRecvReqProc(reinterpret_cast<zc_msg_t *>(req), iqsize, repmsg, opsize);
    } else {
        // msg from sysmod(or CModCli), no need check register and keeplive status
        ret = MsgReqProc(reinterpret_cast<zc_msg_t *>(req), iqsize, repmsg, opsize);
    }

    BuildRepMsgHdr(repmsg, reqmsg);
    repmsg->err = ret;
    if (ret < 0) {
        // error just replay hdr
        *opsize = sizeof(zc_msg_t);
        LOG_ERROR("proc error ret:%d, id:%hu,%hu, pid:%d,modid:%u", ret, reqmsg->id, reqmsg->sid, reqmsg->pid,
                  reqmsg->modid);
    }
    LOG_TRACE("proc ret:%d, id:%hu,%hu, pid:%d,modid:%u", ret, reqmsg->id, reqmsg->sid, reqmsg->pid, reqmsg->modid);

    return ret;
}

bool CModSysBase::registerInsert(zc_msg_t *msg) {
    ZC_U64 key = ((ZC_U64)msg->pid << 32) | msg->modid;
    std::lock_guard<std::mutex> locker(m_mutex);
    auto it = m_modmap.find(key);
    if (it != m_modmap.end()) {
        // already register update;do noting
        LOG_WARN("mod register update,[%s] pid:%d,modid:%u,regtime:%u,last:%u", it->second->pname, it->second->pid,
                 it->second->modid, it->second->regtime, it->second->lasttime);
    } else {
        // insert
        zc_mod_reg_t *reg = reinterpret_cast<zc_mod_reg_t *>(msg->data);
        std::shared_ptr<sys_modcli_status_t> cli(new sys_modcli_status_t());
        cli->status = MODCLI_STATUS_REGISTERED_E;
        time_t now = time(NULL);
        cli->regtime = now;
        cli->lasttime = now;
        cli->modid = msg->modid;
        cli->pid = msg->pid;
        strncpy(cli->pname, reg->pname, sizeof(cli->pname) - 1);
        strncpy(cli->url, reg->url, sizeof(cli->url) - 1);
        LOG_INFO("mod register, [%s]pid:%d,modid:%u,regtime:%u,last:%u, url:%s", cli->pname, cli->pid, cli->modid,
                 cli->regtime, cli->lasttime, cli->url);
        m_modmap.insert(std::make_pair(key, cli));
    }

    return true;
}

bool CModSysBase::unregisterRemove(zc_msg_t *msg) {
    ZC_U64 key = ((ZC_U64)msg->pid << 32) | msg->modid;
    std::lock_guard<std::mutex> locker(m_mutex);
    auto it = m_modmap.find(key);
    if (it == m_modmap.end()) {
        return false;
    }
    LOG_INFO("mod unregister remove, [%s]pid:%d,modid:%u,regtime:%u,last:%u", it->second->pname, it->second->pid,
             it->second->modid, it->second->regtime, it->second->lasttime);
    m_modmap.erase(it);

    return true;
}

bool CModSysBase::updateStatus(zc_msg_t *msg) {
    ZC_U64 key = ((ZC_U64)msg->pid << 32) | msg->modid;
    std::lock_guard<std::mutex> locker(m_mutex);
    auto it = m_modmap.find(key);
    if (it != m_modmap.end()) {
        time_t now = time(NULL);
        // update last msg time
        it->second->lasttime = now;
        return true;
    }

    return false;
}

// sysmod check modcli status; check keepalive timeout
int CModSysBase::_sysCheckModCliStatus() {
    time_t now = time(NULL);
    std::lock_guard<std::mutex> locker(m_mutex);
    for (auto it = m_modmap.begin(); it != m_modmap.end();) {
        if (now > it->second->lasttime + ZC_MOD_KIEEPALIVE_TIME) {
            LOG_ERROR("mod timeout remove, [%s]pid:%d,modid:%u,regtime:%u,last:%u", it->second->pname, it->second->pid,
                      it->second->modid, it->second->regtime, it->second->lasttime);
            // remove it
            it = m_modmap.erase(it);
            // TODO(zhoucc): callback to Mgr
        } else {
            ++it;
        }
    }

    return 0;
}

int CModSysBase::modprocess() {
    // unsigned int ret = 0;
    // unsigned int errcnt = 0;
    LOG_WARN("sys process into[%d] into", m_modid);
    while (State() == Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // LOG_INFO("process sleep[%s]", m_name);
        _sysCheckModCliStatus();
    }
    LOG_WARN("sys process into[%d] exit", m_modid);
    return -1;
}
}  // namespace zc
