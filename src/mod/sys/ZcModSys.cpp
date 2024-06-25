// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <functional>

#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg.h"
#include "zc_msg_sys.h"
#include "zc_type.h"

#include "ZcType.hpp"
#include "sys/ZcModSys.hpp"
#include "sys/ZcMsgProcModSys.hpp"

namespace zc {
CModSys::CModSys()
    : CModBase(ZC_MODID_SYS_E), m_pMsgProc(new CMsgProcModSys()), m_init(false) {}

CModSys::~CModSys() {
    UnInit();
    ZC_SAFE_DELETE(m_pMsgProc);
}

bool CModSys::Init(void *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    if (!m_pMsgProc->Init(cbinfo)) {
        LOG_TRACE("m_pMsgProc Init error");
        goto _err;
    }

    // register msg proc module
    if (!registerMsgProcMod(m_pMsgProc)) {
        LOG_TRACE("registerMsgProcMod error");
        goto _err;
    }

    if (!init()) {
        LOG_TRACE("CModBase init");
        goto _err;
    }

    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    unInit();
    unregisterMsgProcMod(m_pMsgProc);
    m_pMsgProc->UnInit();
    LOG_TRACE("Init error");
    return false;
}

bool CModSys::_unInit() {
    if (!m_init) {
        return true;
    }

    unInit();
    unregisterMsgProcMod(m_pMsgProc);
    m_pMsgProc->UnInit();

    return false;
}

bool CModSys::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}

}  // namespace zc
