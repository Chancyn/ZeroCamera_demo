// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_log.h"

#include "ZcRtspServer.hpp"
#include "ZcRtspManager.hpp"
#include "ZcType.hpp"

namespace zc {
CRtspManager::CRtspManager() : m_init(false), m_running(0) {}

CRtspManager::~CRtspManager() {
    UnInit();
}

bool CRtspManager::Init(rtsp_callback_info_t *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    if (!CModRtsp::Init(cbinfo)) {
        LOG_TRACE("CModRtsp Init error");
        goto _err;
    }

    if (!CRtspServer::Init()) {
        LOG_TRACE("CModRtsp Init error");
        goto _err;
    }

    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    _unInit();

    LOG_TRACE("Init error");
    return false;
}

bool CRtspManager::_unInit() {
    Stop();
    CModRtsp::UnInit();
    CRtspServer::UnInit();
    return false;
}

bool CRtspManager::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}
bool CRtspManager::Start() {
    if (m_running) {
        return false;
    }

    CModRtsp::Start();
    CRtspServer::Start();
    m_running = true;
    return true;
}

bool CRtspManager::Stop() {
    if (!m_running) {
        return false;
    }

    CModRtsp::Stop();
    CRtspServer::Stop();
    m_running = false;
    return true;
}
}  // namespace zc
