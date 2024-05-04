// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
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
CModBase::CModBase(ZC_U8 modid, const char *url) : m_init(false), m_modid(modid), m_seqno(0) {
    strncpy(m_url, url, sizeof(m_url) - 1);
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

const char *g_Modurltab[ZC_MODID_BUTT] = {
    ZC_SYS_URL_IPC,
    ZC_CODEC_URL_IPC,
    ZC_RTSP_URL_IPC,
};

const char *CModBase::GetUrlbymodid(ZC_U8 modid) {
    if (modid >= ZC_MODID_BUTT) {
        return nullptr;
    }
    return g_Modurltab[modid];
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
}  // namespace zc
