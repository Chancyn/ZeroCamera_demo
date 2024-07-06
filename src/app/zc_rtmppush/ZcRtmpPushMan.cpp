// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcRtmpPushMan.hpp"
#include "ZcType.hpp"

namespace zc {
// modsyscli
CRtmpPushMan::CRtmpPushMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0) {}

CRtmpPushMan::~CRtmpPushMan() {
    UnInit();
}

bool CRtmpPushMan::Init(unsigned int type, unsigned int chn, const char *url) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }
    zc_stream_info_t info;

    if (sendSMgrGetInfo(type, chn, &info) < 0) {
        LOG_TRACE("sendSMgrGetInfo error");
        goto _err;
    }

    if (!CRtmpPush::Init(info, url)) {
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
}  // namespace zc

bool CRtmpPushMan::_unInit() {
    Stop();
    CRtmpPush::UnInit();

    return false;
}

bool CRtmpPushMan::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}
bool CRtmpPushMan::Start() {
    if (m_running) {
        return false;
    }

    m_running = CRtmpPush::StartCli();
    return m_running;
}

bool CRtmpPushMan::Stop() {
    if (!m_running) {
        return false;
    }

    CRtmpPush::StopCli();
    m_running = false;
    return true;
}
}  // namespace zc
