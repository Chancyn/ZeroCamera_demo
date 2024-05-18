// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <functional>

#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"
#include "zc_msg_codec.h"
#include "zc_msg_rtsp.h"
#include "zc_msg_sys.h"
#include "zc_type.h"

#include "ZcModBase.hpp"
#include "ZcType.hpp"

namespace zc {
const char *g_Modurltab[ZC_MODID_BUTT] = {
    ZC_SYS_URL_IPC,
    ZC_CODEC_URL_IPC,
    ZC_RTSP_URL_IPC,
};

const char *g_Modnametab[ZC_MODID_BUTT] = {
    ZC_SYS_MODNAME,
    ZC_CODEC_MODNAME,
    ZC_RTSP_MODNAME,
};

static inline const char *get_url_bymodid(ZC_U8 modid) {
    if (modid >= ZC_MODID_BUTT) {
        return nullptr;
    }
    return g_Modurltab[modid];
}

static inline const char *get_name_bymodid(ZC_U8 modid) {
    if (modid >= ZC_MODID_BUTT) {
        return nullptr;
    }
    return g_Modnametab[modid];
}

const char *CModBase::GetUrlbymodid(ZC_U8 modid) {
    return get_url_bymodid(modid);
}

CModBase::CModBase(ZC_U8 modid)
    : Thread(std::string(get_name_bymodid(modid))), m_init(false), m_status(false), m_modid(modid), m_seqno(0) {
    strncpy(m_url, get_url_bymodid(modid), sizeof(m_url) - 1);
    strncpy(m_name, get_name_bymodid(modid), sizeof(m_name) - 1);
}

CModBase::~CModBase() {
    unInit();
}

bool CModBase::init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    auto svrreq = std::bind(&CModBase::_svrRecvReqProc, this, std::placeholders::_1, std::placeholders::_2,
                            std::placeholders::_3, std::placeholders::_4);

    if (!InitComm(m_url, svrreq)) {
        LOG_ERROR("InitComm error");
        goto _err;
    }

    LOG_TRACE("init ok");
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

ZC_S32 CModBase::_svrRecvReqProc(char *req, int iqsize, char *rep, int *opsize) {
    // TODO(zhoucc) find msg procss mod
    if (m_pmsgmodproc) {
        return m_pmsgmodproc->MsgReqProc(reinterpret_cast<zc_msg_t *>(req), iqsize, reinterpret_cast<zc_msg_t *>(rep),
                                         opsize);
    }

    return 0;
}

ZC_S32 CModBase::_cliRecvRepProc(char *rep, int size) {
    // TODO(zhoucc) find msg procss mod

    return 0;
}

bool CModBase::BuildReqMsgHdr(zc_msg_t *pmsg, ZC_U8 modidto, ZC_U16 id, ZC_U16 sid, ZC_U8 chn, ZC_U32 size) {
    if (!pmsg) {
        pmsg = reinterpret_cast<zc_msg_t *>(new char[sizeof(zc_msg_t) + size]());
    }

    pmsg->ver = ZC_MSG_VERSION;
    pmsg->modidto = modidto;
    pmsg->modid = m_modid;
    pmsg->msgtype = ZC_MSG_TYPE_REQ_E;
    pmsg->chn = chn;
    pmsg->id = id;
    pmsg->sid = sid;
    pmsg->size = size;
    pmsg->err = 0;

    return true;
}

bool CModBase::MsgSendTo(zc_msg_t *pmsg, const char *urlto) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    pmsg->ts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    pmsg->seq = m_seqno++;
    CMsgCommReqClient cli;
    cli.Open(urlto);
    cli.Send(pmsg, sizeof(zc_msg_t) + pmsg->size, 0);

    return true;
}

bool CModBase::MsgSendTo(zc_msg_t *pmsg) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    pmsg->ts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    pmsg->seq = m_seqno++;
    CMsgCommReqClient cli;
    cli.Open(GetUrlbymodid(pmsg->modidto));
    cli.Send(pmsg, sizeof(zc_msg_t) + pmsg->size, 0);

    return true;
}

int CModBase::_sendRegisterMsg() {
    if (m_modid == ZC_MODID_SYS_E) {
        return -1;
    }

    LOG_TRACE("send register msg into[%s] into", m_name);
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_reg_t)] = {0};
    zc_msg_t *pmsg = reinterpret_cast<zc_msg_t *>(msg_buf);

    BuildReqMsgHdr(pmsg, ZC_MODID_SYS_E, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_REGISTER_E, 0, sizeof(zc_mod_reg_t));
    MsgSendTo(pmsg, ZC_SYS_URL_IPC);
    // MsgSendTo(pmsg);

    return 0;
}

// send keepalive
int CModBase::_sendKeepaliveMsg() {
    if (m_modid == ZC_MODID_SYS_E) {
        return -1;
    }
    static ZC_U32 s_seqno = 0;
    LOG_TRACE("send keepalive msg into[%s] into", m_name);
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_keepalive_t)] = {0};
    zc_msg_t *pmsg = reinterpret_cast<zc_msg_t *>(msg_buf);
    zc_mod_keepalive_t *pkeepalive = reinterpret_cast<zc_mod_keepalive_t *>(pmsg->data);
    pkeepalive->seqno = s_seqno++;
    pkeepalive->status = m_status;
    pkeepalive->mid = m_modid;
    BuildReqMsgHdr(pmsg, ZC_MODID_SYS_E, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_KEEPALIVE_E, 0, sizeof(zc_mod_keepalive_t));
    MsgSendTo(pmsg, ZC_SYS_URL_IPC);
    LOG_TRACE("send keepalive msg into id[%d] sid[%d] into", pmsg->id, pmsg->sid);
    // MsgSendTo(pmsg);

    return 0;
}

int CModBase::_process_sys() {
    // unsigned int ret = 0;
    // unsigned int errcnt = 0;
    LOG_WARN("process into[%s] into", m_name);
    while (State() == Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        LOG_INFO("process sleep[%s]", m_name);
    }
    LOG_WARN("process into[%s] exit", m_name);
    return -1;
}

int CModBase::_process_mod() {
    // unsigned int ret = 0;
    // unsigned int errcnt = 0;
    LOG_WARN("process into[%s] into", m_name);
    _sendRegisterMsg();
    while (State() == Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        LOG_INFO("process sleep[%s]", m_name);
        _sendKeepaliveMsg();
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
