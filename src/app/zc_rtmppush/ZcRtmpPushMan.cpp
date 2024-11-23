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
CRtmpPushMan::CRtmpPushMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0), m_push(nullptr) {}

CRtmpPushMan::~CRtmpPushMan() {
    UnInit();
}

bool CRtmpPushMan::Init(unsigned int type, unsigned int chn, const char *url) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    m_push = CIRtmpPushFac::CreateRtmpPush(ZC_RTMPPUSH_AIO_E);
    //push = CIRtmpPushFac::CreateRtmpPush(ZC_RTMPPUSH_E);
    if (!m_push) {
        LOG_TRACE("CreateRtmpPush error");
        return false;
    }

    zc_stream_info_t info;
    if (sendSMgrGetInfo(type, chn, &info) < 0) {
        LOG_TRACE("sendSMgrGetInfo error");
        goto _err;
    }

    if (!m_push->Init(info, url)) {
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

    if (m_push) {
        m_push->UnInit();
        delete m_push;
    }

    return true;
}

bool CRtmpPushMan::UnInit() {
    if (!m_init) {
        return false;
    }

    _unInit();

    m_init = false;
    return true;
}

bool CRtmpPushMan::Start() {
    if (m_running) {
        return false;
    }

    if (m_push) {
        m_running = m_push->StartCli();
    }

    return m_running;
}

bool CRtmpPushMan::Stop() {
    if (!m_running) {
        return false;
    }

    m_push->StopCli();
    m_running = false;
    return true;
}
}  // namespace zc
