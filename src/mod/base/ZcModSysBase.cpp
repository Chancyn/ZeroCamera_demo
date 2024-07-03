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
CModSysBase::CModSysBase() : CModBase(ZC_MODID_SYS_E), CModPublish(ZC_MODID_SYS_E) {}

CModSysBase::~CModSysBase() {}

bool CModSysBase::initPubSvr() {
    if (!CModPublish::InitPub()) {
        LOG_ERROR("initPubSvr error Open");
        return false;
    }

    LOG_TRACE("initPubSvr ok");
    return true;
}

bool CModSysBase::unInitPubSvr() {
    CMsgCommPubServer::Close();

    return true;
}

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

    if (!initPubSvr()) {
        LOG_ERROR("PubSvr init error modid:%d, url:%s", m_modid, m_url);
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
    unInitPubSvr();
    m_init = false;
    LOG_TRACE("unInit ok");
    return false;
}

bool CModSysBase::Start() {
    CModBase::Start();
    // CModSubscriber::Start();

    return true;
}

bool CModSysBase::Stop() {
    // CModSubscriber::Stop();
    CModBase::Stop();
    return true;
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
        // msg from sysmod(or CModReqCli), no need check register and keeplive status
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
    std::shared_ptr<sys_modcli_status_t> cli;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        auto it = m_modmap.find(key);
        if (it != m_modmap.end()) {
            cli = it->second;
            // already register update;do noting
            LOG_WARN("mod register update,[%s] pid:%d,modid:%u,regtime:%u,last:%u", it->second->pname, it->second->pid,
                     it->second->modid, it->second->regtime, it->second->lasttime);
        } else {
            // insert
            zc_mod_reg_t *reg = reinterpret_cast<zc_mod_reg_t *>(msg->data);
            cli.reset(new sys_modcli_status_t());
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
    }

    PublishRegister(MODCLI_STATUS_REGISTERED_E, cli.get());
    return true;
}

bool CModSysBase::unregisterRemove(zc_msg_t *msg) {
    ZC_U64 key = ((ZC_U64)msg->pid << 32) | msg->modid;
    std::shared_ptr<sys_modcli_status_t> cli;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        auto it = m_modmap.find(key);
        if (it == m_modmap.end()) {
            return false;
        }
        cli = it->second;
        LOG_INFO("mod unregister remove, [%s]pid:%d,modid:%u,regtime:%u,last:%u", it->second->pname, it->second->pid,
                 it->second->modid, it->second->regtime, it->second->lasttime);
        m_modmap.erase(it);
    }
    // pushlish registermsg
    PublishRegister(MODCLI_STATUS_UNREGISTER_E, cli.get());
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
            PublishRegister(MODCLI_STATUS_EXPIRED_E, it->second.get());
            it = m_modmap.erase(it);
            // TODO(zhoucc): callback to Mgr
        } else {
            ++it;
        }
    }

    return 0;
}

// send get streaminfo
int CModSysBase::PublishRegister(int regstatus, sys_modcli_status_t *info) {
    LOG_TRACE("PublishRegister");
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_pub_reg_t)] = {0};
    zc_msg_t *sub = reinterpret_cast<zc_msg_t *>(msg_buf);
    BuildPubMsgHdr(sub, ZC_PUBMID_SYS_MAN, ZC_PUBMSID_SYS_MAN_REG, 0, sizeof(zc_mod_pub_reg_t));
    zc_mod_pub_reg_t *subinfo = reinterpret_cast<zc_mod_pub_reg_t *>(sub->data);
    subinfo->regstatus = regstatus;  // MODCLI_STATUS_REGISTERED_E;
    subinfo->regtime = info->regtime;
    subinfo->lasttime = info->lasttime;
    subinfo->modid = info->modid;
    subinfo->pid = info->pid;
    strncpy(subinfo->pname, subinfo->pname, sizeof(subinfo->pname) - 1);
    strncpy(subinfo->url, subinfo->url, sizeof(subinfo->url) - 1);

    LOG_INFO("pub register, [%s]pid:%d,modid:%u,regtime:%u,last:%u, url:%s", subinfo->pname, subinfo->pid,
             subinfo->modid, subinfo->regtime, subinfo->lasttime, subinfo->url);
    if (!Publish(sub, sizeof(msg_buf))) {
        LOG_ERROR("Publish register err:%d \n");
    }
    LOG_TRACE("PublishRegister ok");
    return 0;
}

// send get streaminfo
int CModSysBase::PublishStreamUpdate(unsigned int chn, unsigned int type) {
    LOG_TRACE("PublishStreamUpdate");
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_pub_streamupdate_t)] = {0};
    zc_msg_t *sub = reinterpret_cast<zc_msg_t *>(msg_buf);
    BuildPubMsgHdr(sub, ZC_PUBMID_SYS_MAN, ZC_PUBMSID_SYS_MAN_STREAM_UPDATE, 0, sizeof(zc_mod_pub_streamupdate_t));
    zc_mod_pub_streamupdate_t *subinfo = reinterpret_cast<zc_mod_pub_streamupdate_t *>(sub->data);
    subinfo->type = type;
    subinfo->chn = chn;

    LOG_INFO("pub streamupdate, [%s]pid:%d,modid:%u,regtime:%u,last:%u, url:%s", subinfo->type, subinfo->chn);
    if (!Publish(sub, sizeof(msg_buf))) {
        LOG_ERROR("Publish streamupdate err:%d \n");
    }
    LOG_TRACE("PublishStreamUpdate ok");
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
