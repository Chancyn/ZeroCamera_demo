// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <functional>

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"

#include "ZcModBase.hpp"
#include "zc_type.h"

namespace zc {
CModBase::CModBase(ZC_U8 modid, const char *url) : m_init(false), m_modid(modid) {
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
}  // namespace zc
