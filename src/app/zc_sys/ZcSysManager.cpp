// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_log.h"

#include "ZcSysManager.hpp"
#include "ZcType.hpp"

namespace zc {
CSysManager::CSysManager() : m_init(false), m_running(0) {}

CSysManager::~CSysManager() {
    _unInit();
}

bool CSysManager::Init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    if (!CModSys::Init()) {
        LOG_TRACE("CModRtsp Init error");
        goto _err;
    }

    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    unInit();

    LOG_TRACE("Init error");
    return false;
}

bool CSysManager::_unInit() {
    if (!m_init) {
        return true;
    }
    Stop();
    CModSys::UnInit();
    return false;
}

bool CSysManager::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}

bool CSysManager::Start() {
    if (m_running) {
        return false;
    }

    CModSys::Start();

    m_running = true;
    return true;
}

bool CSysManager::Stop() {
    if (!m_running) {
        return false;
    }

    CModSys::Stop();

    m_running = false;
    return true;
}
}  // namespace zc
