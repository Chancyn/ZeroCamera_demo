// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <functional>

#include "MsgCommSubClient.hpp"
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
#include "ZcModSubBase.hpp"
#include "ZcType.hpp"

namespace zc {
CModSubBase::CModSubBase(ZC_U8 modid) : CModBase(modid), CModSubscriber(modid), m_init(false), m_status(false) {}

CModSubBase::~CModSubBase() {}

bool CModSubBase::initSubCli() {
    auto subrecv = std::bind(&CModSubBase::subcliRecvProc, this, std::placeholders::_1, std::placeholders::_2);

    if (!CModSubscriber::InitSub(subrecv)) {
        LOG_ERROR("initPubSvr error Open");
        return false;
    }

    if (!CModSubscriber::Start()) {
        LOG_ERROR("initPubSvr error start");
        return false;
    }
    LOG_TRACE("initPubSvr ok");
    return true;
}

bool CModSubBase::unInitSubCli() {
    CMsgCommSubClient::Close();

    return true;
}

bool CModSubBase::init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    auto svrreq = std::bind(&CModSubBase::reqSvrRecvReqProc, this, std::placeholders::_1, std::placeholders::_2,
                            std::placeholders::_3, std::placeholders::_4);

    if (!initReqSvr(svrreq)) {
        LOG_ERROR("InitComm error modid:%d, url:%s", m_modid, m_url);
        goto _err;
    }

    if (!initSubCli()) {
        LOG_ERROR("PubCli init error modid:%d", m_modid);
        goto _err;
    }

    m_init = true;
    LOG_TRACE("init ok modid:%d, url:%s", m_modid, m_url);
    return true;

_err:
    LOG_ERROR("Init error");
    return false;
}

bool CModSubBase::unInit() {
    if (!m_init) {
        return true;
    }

    unInitReqSvr();
    m_init = false;
    LOG_TRACE("unInit ok");
    return false;
}

bool CModSubBase::Start() {
    CModBase::Start();
    CModSubscriber::Start();

    return true;
}

bool CModSubBase::Stop() {
    CModSubscriber::Stop();
    CModBase::Stop();
    return true;
}

int CModSubBase::subcliRecvProc(char *req, int iqsize) {
    int ret = 0;
    zc_msg_t *submsg = reinterpret_cast<zc_msg_t *>(req);
    CModBase::DumpModMsg(*submsg);
    // ret = MsgSubProc(submsg, iqsize);
    if (ret < 0) {
        LOG_ERROR("proc error ret:%d, id:%hu,%hu, pid:%d,modid:%u", ret, submsg->id, submsg->sid, submsg->pid,
                  submsg->modid);
        return 0;
    }
    LOG_TRACE("proc ret:%d, id:%hu,%hu, pid:%d,modid:%u", ret, submsg->id, submsg->sid, submsg->pid, submsg->modid);

    return 0;
}

int CModSubBase::reqSvrRecvReqProc(char *req, int iqsize, char *rep, int *opsize) {
    int ret = 0;
    zc_msg_t *reqmsg = reinterpret_cast<zc_msg_t *>(req);
    zc_msg_t *repmsg = reinterpret_cast<zc_msg_t *>(rep);
    ret = MsgReqProc(reqmsg, iqsize, repmsg, opsize);
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

// send keepalive
int CModSubBase::_sendRegisterMsg(int cmd) {
    // LOG_TRACE("send register msg into[%d] into", m_modid);
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
    LOG_TRACE("send register:%d pid:%d,modid:%d, pname:%s,date:%s, cos1:%llu,%llu", reqreg->regcmd, req->pid,
              req->modid, reqreg->pname, reqreg->date, (rep->ts1 - rep->ts), (now - rep->ts));
#endif
    return 0;
}

// send keepalive
int CModSubBase::_sendKeepaliveMsg() {
    static ZC_U32 s_seqno = 0;
    // LOG_TRACE("send keepalive msg into[%d] into", m_modid);
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

int CModSubBase::modprocess() {
    // unsigned int ret = 0;
    // unsigned int errcnt = 0;
    LOG_WARN("submod:%u process into into", m_modid);
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

    LOG_WARN("submod:%u process exit", m_modid);
    return -1;
}

}  // namespace zc
