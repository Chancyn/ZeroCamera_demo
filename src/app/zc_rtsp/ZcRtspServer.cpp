// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_log.h"

#include "ZcRtspServer.hpp"
#include "ZcType.hpp"

namespace zc {
CRtspServer::CRtspServer() : m_init(false), m_running(0) {}

CRtspServer::~CRtspServer() {
    _unInit();
}

bool CRtspServer::Init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    if (!CModRtsp::Init()) {
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

bool CRtspServer::_unInit() {
    if (!m_init) {
        return true;
    }

    Stop();
    CModRtsp::UnInit();
    return false;
}

bool CRtspServer::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}
bool CRtspServer::Start() {
    if (m_running) {
        return false;
    }

    CModRtsp::Start();
    m_running = true;
    return true;
}

bool CRtspServer::Stop() {
    if (!m_running) {
        return false;
    }

    CModRtsp::Stop();
    m_running = false;
    return true;
}
}  // namespace zc
