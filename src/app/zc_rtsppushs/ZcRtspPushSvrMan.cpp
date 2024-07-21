// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "ZcModCli.hpp"
#include "ZcRtspPushServer.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"
#include "zc_stream_mgr.h"

#include "ZcRtspPushSvrMan.hpp"
#include "ZcType.hpp"

namespace zc {
// modsyscli
CRtspPushSvrMan::CRtspPushSvrMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0) {}

CRtspPushSvrMan::~CRtspPushSvrMan() {
    UnInit();
}

int CRtspPushSvrMan::_getStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli get info chn:%d", chn);
    // TODO(zhoucc): cb
    int ret = sendSMgrGetInfo(ZC_SHMSTREAM_PUSHS, chn, info);
    if (ret < 0) {
        return ret;
    }
    memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
    return ret;
}

int CRtspPushSvrMan::getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CRtspPushSvrMan *pRtsp = reinterpret_cast<CRtspPushSvrMan *>(ptr);
    return pRtsp->_getStreamInfoCb(chn, info);
}

int CRtspPushSvrMan::_setStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli set info type:%u, chn:%d", chn);
    // TODO(zhoucc): cb
    if (memcmp(&m_mediainfo, info, sizeof(zc_stream_info_t)) != 0) {
        memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
        return sendSMgrSetInfo(ZC_SHMSTREAM_PUSHS, chn, &m_mediainfo);
    }

    return 0;
}

int CRtspPushSvrMan::setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CRtspPushSvrMan *pRtsp = reinterpret_cast<CRtspPushSvrMan *>(ptr);
    return pRtsp->_setStreamInfoCb(chn, info);
}

bool CRtspPushSvrMan::Init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    rtsppushs_callback_info_t cbinfo = {
        .GetInfoCb = getStreamInfoCb,
        .SetInfoCb = setStreamInfoCb,
        .MgrContext = this,
    };

    if (!CRtspPushServer::Init(&cbinfo)) {
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

bool CRtspPushSvrMan::_unInit() {
    Stop();
    CRtspPushServer::UnInit();

    return true;
}

bool CRtspPushSvrMan::UnInit() {
    if (!m_init) {
        return false;
    }

    _unInit();

    m_init = false;
    return true;
}
bool CRtspPushSvrMan::Start() {
    if (m_running) {
        return false;
    }
    CRtspPushServer::Start();
    m_running = true;

    return m_running;
}

bool CRtspPushSvrMan::Stop() {
    if (!m_running) {
        return false;
    }

    CRtspPushServer::Start();
    m_running = false;
    return true;
}
}  // namespace zc
