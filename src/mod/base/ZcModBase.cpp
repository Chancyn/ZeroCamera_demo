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
#include "ZcModCli.hpp"
#include "ZcType.hpp"

namespace zc {

static inline void BuildRepMsgHdr(zc_msg_t *rep, zc_msg_t *req) {
    memcpy(rep, req, sizeof(zc_msg_t));
    rep->msgtype = ZC_MSG_TYPE_REP_E;
    rep->ts1 = zc_system_time();
    return;
}

CModBase::CModBase(ZC_U8 modid, ZC_U32 version)
    : CModCli(modid), Thread(std::string(CModCli::GetUrlbymodid(modid))), m_init(false), m_status(false),
      m_modid(modid), m_seqno(0), m_version(version) {
    m_pid = getpid();
    strncpy(m_url, CModCli::GetUrlbymodid(modid), sizeof(m_url) - 1);
    strncpy(m_name, CModCli::GetUrlbymodid(modid), sizeof(m_name) - 1);
    ZC_PROC_GETNAME(m_pname, sizeof(m_pname));
}

CModBase::~CModBase() {
    unInit();
}

int CModBase::_checklicense() {
    if (unlikely(m_syslicstatus == SYS_LIC_STATUS_TEMP_LIC_E)) {
        time_t now = time(NULL);
        if (now > m_expire) {
            m_syslicstatus = SYS_LIC_STATUS_EXPIRED_LIC_E;
            LOG_ERROR("license timeout now:%u > %u", now, m_expire);
        }
    }

    return m_syslicstatus;
}

void CModBase::_initlicense() {
    // TODO(zhoucc): load license
    m_inittime = time(NULL);
    m_syslicstatus = SYS_LIC_STATUS_SUC_E;
    m_expire = m_inittime + ZC_MOD_LIC_EXPIRE_TIME;
    LOG_TRACE("modid:%d init license:%d", m_modid, m_syslicstatus);
    ZC_ASSERT(m_syslicstatus != SYS_LIC_STATUS_ERR_E);

    return;
}

bool CModBase::init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // init license
    _initlicense();

    MsgCommReqSerHandleCb svrreq = nullptr;
    if (m_modid == ZC_MODID_SYS_E) {
        svrreq = std::bind(&CModBase::svrSysRecvReqProc, this, std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3, std::placeholders::_4);
    } else {
        svrreq = std::bind(&CModBase::svrRecvReqProc, this, std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3, std::placeholders::_4);
    }

    if (!InitComm(m_url, svrreq)) {
        LOG_ERROR("InitComm error modid:%d, url:%s", m_modid, m_url);
        goto _err;
    }

    LOG_TRACE("init ok modid:%d, url:%s", m_modid, m_url);
    return true;

_err:
    LOG_ERROR("Init error");
    return false;
}

bool CModBase::unInit() {
    if (!m_init) {
        return true;
    }

    UnInitComm();
    LOG_TRACE("unInit ok");
    return false;
}

bool CModBase::registerMsgProcMod(CMsgProcMod *msgprocmod) {
    // uinit status can register msgprocmod
    if (m_init) {
        return false;
    }

    // TODO(zhoucc) support register one or more msgprocmod
    m_pmsgmodproc = msgprocmod;

    return true;
}

bool CModBase::unregisterMsgProcMod(CMsgProcMod *msgprocmod) {
    // uinit status can register msgprocmod
    if (m_init) {
        return false;
    }

    // TODO(zhoucc) : support more msgprocmod
    m_pmsgmodproc = nullptr;

    return false;
}

// sys mod
zc_msg_errcode_e CModBase::_svrSysCheckRecvReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    if (unlikely(_checklicense() < 0)) {
        LOG_TRACE("mod license error pid:%d,modid:%u", req->pid, req->modid);
        return ZC_MSG_ERR_LICENSE_E;
    }

    int ret = ZC_MSG_ERR_E;
    static ZC_U32 registerkey = (ZC_MID_SYS_MAN_E << 16) | ZC_MSID_SYS_MAN_REGISTER_E;
    // TODO(zhoucc) find msg procss mod
    if (m_pmsgmodproc) {
        ZC_U32 key = (req->id << 16) | req->sid;
        if (likely(key != registerkey)) {
            if (updateStatus(req)) {
                ret = m_pmsgmodproc->MsgReqProc(req, iqsize, rep, opsize);
            } else {
                ret = ZC_MSG_ERR_UNREGISTER_E;
            }
        } else {
            zc_mod_reg_t *reqreg = reinterpret_cast<zc_mod_reg_t *>(req->data);
            ret = m_pmsgmodproc->MsgReqProc(req, iqsize, rep, opsize);
            if (ret >= 0) {
                if (reqreg->regcmd == ZC_SYS_REGISTER_E) {
                    registerInsert(req);
                } else if (reqreg->regcmd == ZC_SYS_UNREGISTER_E) {
                    // handle error or unregister
                    unregisterRemove(req);
                }
            }
        }
    }

    return (zc_msg_errcode_e)ret;
}

// sys reqproc
ZC_S32 CModBase::svrSysRecvReqProc(char *req, int iqsize, char *rep, int *opsize) {
    int ret = 0;
    zc_msg_t *reqmsg = reinterpret_cast<zc_msg_t *>(req);
    zc_msg_t *repmsg = reinterpret_cast<zc_msg_t *>(rep);

    if (reqmsg->modid != ZC_MODID_SYS_E) {
        // msg not from sysmod, check register and keeplive status
        ret = _svrSysCheckRecvReqProc(reinterpret_cast<zc_msg_t *>(req), iqsize, repmsg, opsize);
    } else {
        // msg from sysmod(or CModCli), no need check register and keeplive status
        ret = _svrRecvReqProc(reinterpret_cast<zc_msg_t *>(req), iqsize, repmsg, opsize);
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

zc_msg_errcode_e CModBase::_svrRecvReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    // TODO(zhoucc) find msg procss mod
    if (m_pmsgmodproc) {
        return (zc_msg_errcode_e)m_pmsgmodproc->MsgReqProc(req, iqsize, rep, opsize);
    }

    return ZC_MSG_ERR_E;
}

int CModBase::svrRecvReqProc(char *req, int iqsize, char *rep, int *opsize) {
    int ret = 0;
    zc_msg_t *reqmsg = reinterpret_cast<zc_msg_t *>(req);
    zc_msg_t *repmsg = reinterpret_cast<zc_msg_t *>(rep);
    ret = _svrRecvReqProc(reqmsg, iqsize, repmsg, opsize);
    BuildRepMsgHdr(repmsg, reqmsg);
    repmsg->err = ret;
    if (ret < 0) {
        // error just replay hdr
        *opsize = sizeof(zc_msg_t);
        LOG_ERROR("proc error ret:%d, id:%hu,%hu, pid:%d,modid:%u", ret, reqmsg->id, reqmsg->sid, reqmsg->pid,
                  reqmsg->modid);
        return 0;
    }
    LOG_TRACE("proc ret:%d, id:%hu,%hu, pid:%d,modid:%u", ret, reqmsg->id, reqmsg->sid, reqmsg->pid, reqmsg->modid);

    return 0;
}

ZC_S32 CModBase::_cliRecvRepProc(char *rep, int size) {
    // TODO(zhoucc) find msg procss mod

    return 0;
}

// send keepalive
int CModBase::_sendRegisterMsg(int cmd) {
    if (m_modid == ZC_MODID_SYS_E) {
        return -1;
    }

    // LOG_TRACE("send register msg into[%s] into", m_name);
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_reg_t)] = {0};
    zc_msg_t *req = reinterpret_cast<zc_msg_t *>(msg_buf);
    BuildReqMsgHdr(req, ZC_MODID_SYS_E, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_REGISTER_E, 0, sizeof(zc_mod_reg_t));
    zc_mod_reg_t *reqreg = reinterpret_cast<zc_mod_reg_t *>(req->data);
    reqreg->regcmd = cmd;
    reqreg->ver = m_version;
    strncpy(reqreg->date, g_buildDateTime, sizeof(reqreg->date) - 1);
    strncpy(reqreg->pname, m_pname, sizeof(reqreg->pname) - 1);
    strncpy(reqreg->url, m_url, sizeof(reqreg->url) - 1);

    // recv
    char rmsg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_reg_t)] = {0};
    zc_msg_t *rep = reinterpret_cast<zc_msg_t *>(rmsg_buf);
    size_t rlen = sizeof(zc_msg_t) + sizeof(zc_mod_reg_t);
    zc_mod_reg_t *repreg = reinterpret_cast<zc_mod_reg_t *>(rep->data);
    if (MsgSendTo(req, ZC_SYS_URL_IPC, rep, &rlen)) {
        if (rep->err != 0) {
            LOG_ERROR("recv register rep err:%d \n", rep->err);
        }
    }
#if ZC_DEBUG
    uint64_t now = zc_system_time();
    LOG_TRACE("send register:%d pid:%d,modid:%d, pname:%s,mod:%s,date:%s, cos1:%llu,%llu", reqreg->regcmd, req->pid,
              req->modid, reqreg->pname, m_name, reqreg->date, (rep->ts1 - rep->ts), (now - rep->ts));
#endif
    return 0;
}

// send keepalive
int CModBase::_sendKeepaliveMsg() {
    if (m_modid == ZC_MODID_SYS_E) {
        return -1;
    }
    static ZC_U32 s_seqno = 0;
    // LOG_TRACE("send keepalive msg into[%s] into", m_name);
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_keepalive_t)] = {0};
    zc_msg_t *pmsg = reinterpret_cast<zc_msg_t *>(msg_buf);
    BuildReqMsgHdr(pmsg, ZC_MODID_SYS_E, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_KEEPALIVE_E, 0, sizeof(zc_mod_keepalive_t));
    zc_mod_keepalive_t *pkeepalive = reinterpret_cast<zc_mod_keepalive_t *>(pmsg->data);
    pkeepalive->seqno = s_seqno++;
    pkeepalive->status = m_status;
    strncpy(pkeepalive->date, g_buildDateTime, sizeof(pkeepalive->date) - 1);

    char rmsg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_keepalive_rep_t)] = {0};
    zc_msg_t *prmsg = reinterpret_cast<zc_msg_t *>(rmsg_buf);
    size_t rlen = sizeof(zc_msg_t) + sizeof(zc_mod_keepalive_rep_t);
    zc_mod_keepalive_rep_t *prkeepalive = reinterpret_cast<zc_mod_keepalive_rep_t *>(pmsg->data);
    if (MsgSendTo(pmsg, ZC_SYS_URL_IPC, prmsg, &rlen)) {
        if (prmsg->err != 0) {
            LOG_ERROR("recv keepalive rep err:%d \n", prmsg->err);
        }
        LOG_TRACE("recv keepalive rep success, modid:%d, seq:%u, status:%d,date:%s->%s", pmsg->modid,
                  prkeepalive->seqno, prkeepalive->status, pkeepalive->date, prkeepalive->date);
    }
    // LOG_TRACE("send keepalive msg into id[%d] sid[%d] into", pmsg->id, pmsg->sid);

    return 0;
}

#if 1  // ZC_DEBUG_DUMP
static inline void _dumpTrackInfo(const char *user, zc_meida_track_t *info) {
    LOG_TRACE("[%s] ch:%u,trackno:%u,track:%u,encode:%u,en:%u,size:%u,fmaxlen:%u,status:%u,name:%s", user, info->chn,
              info->trackno, info->tracktype, info->encode, info->enable, info->fifosize, info->framemaxlen,
              info->status, info->name);
    return;
}

static inline void _dumpStreamInfo(const char *user, zc_stream_info_t *info) {
    LOG_TRACE("[%s] type:%d,idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype, info->idx, info->chn,
              info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}
#endif

bool CModBase::registerInsert(zc_msg_t *msg) {
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

bool CModBase::unregisterRemove(zc_msg_t *msg) {
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

bool CModBase::updateStatus(zc_msg_t *msg) {
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
int CModBase::_sysCheckModCliStatus() {
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

int CModBase::_process_mod() {
    // unsigned int ret = 0;
    // unsigned int errcnt = 0;
    LOG_WARN("process into[%s] into", m_name);
    _sendRegisterMsg(ZC_SYS_REGISTER_E);
    // TODO(zhoucc): check register ret
    // sendSMgrGetInfo(0, 0, NULL);
    while (State() == Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // LOG_INFO("process sleep[%s]", m_name);
        _sendKeepaliveMsg();
    }
    // unregister
    _sendRegisterMsg(ZC_SYS_UNREGISTER_E);

    LOG_WARN("process into[%s] exit", m_name);
    return -1;
}

int CModBase::_process_sys() {
    // unsigned int ret = 0;
    // unsigned int errcnt = 0;
    LOG_WARN("process into[%s] into", m_name);
    while (State() == Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // LOG_INFO("process sleep[%s]", m_name);
        _sysCheckModCliStatus();
    }
    LOG_WARN("process into[%s] exit", m_name);
    return -1;
}

int CModBase::process() {
    LOG_INFO("process into[%s] into", m_name);
    if (m_modid != ZC_MODID_SYS_E) {
        _process_mod();
    } else {
        _process_sys();
    }

    LOG_INFO("process into[%s] exit", m_name);
    return -1;
}

}  // namespace zc
