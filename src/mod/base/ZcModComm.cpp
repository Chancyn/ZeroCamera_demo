// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"

#include "ZcModComm.hpp"

namespace zc {
CModComm::CModComm() : m_init(false), m_psvr(new CMsgCommRepServer()) {
}

CModComm::~CModComm() {
    UnInitComm();
    ZC_SAFE_DELETE(m_psvr);
}

bool CModComm::InitComm(const char *url, MsgCommReqSerHandleCb svrcb) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }
    ZC_ASSERT(m_psvr != nullptr);
    if (!m_psvr->Open(url, svrcb)) {
        LOG_ERROR("svr open error");
        return false;
    }

    if (!m_psvr->Start()) {
        LOG_ERROR("svr start error");
        goto _err;
    }

    m_init = true;
    LOG_TRACE("svr open ok");
    return true;

_err:
    m_psvr->Close();
    LOG_ERROR("InitComm error");
    return false;
}

bool CModComm::UnInitComm() {
    if (!m_init) {
        return true;
    }

    m_psvr->Stop();
    m_psvr->Close();
    m_init = false;

    return false;
}
}  // namespace zc
