// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>

#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcRtmpPullMan.hpp"
#include "ZcType.hpp"

namespace zc {
// modsyscli
CRtmpPullMan::CRtmpPullMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0), m_pull(nullptr) {
    memset(&m_mediainfo, 0, sizeof(zc_stream_info_t));
}

CRtmpPullMan::~CRtmpPullMan() {
    UnInit();
}

int CRtmpPullMan::_getStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtmppull get info chn:%d", chn);
    // TODO(zhoucc): cb
    int ret = sendSMgrGetInfo(ZC_SHMSTREAM_PULLC, chn, info);
    if (ret < 0) {
        return ret;
    }
    memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
    return ret;
}

int CRtmpPullMan::getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    return reinterpret_cast<CRtmpPullMan *>(ptr)->_getStreamInfoCb(chn, info);
}

int CRtmpPullMan::_setStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtmppull set info type:%u, chn:%d", chn);
    // TODO(zhoucc): cb
    if (memcmp(&m_mediainfo, info, sizeof(zc_stream_info_t)) != 0) {
        memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
        return sendSMgrSetInfo(ZC_SHMSTREAM_PULLC, chn, &m_mediainfo);
    }

    return 0;
}

int CRtmpPullMan::setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    return  reinterpret_cast<CRtmpPullMan *>(ptr)->_setStreamInfoCb(chn, info);
}

bool CRtmpPullMan::Init(unsigned int chn, const char *url) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    rtmppull_callback_info_t cbinfo = {
        .GetInfoCb = getStreamInfoCb,
        .SetInfoCb = setStreamInfoCb,
        .MgrContext = this,
    };

    m_pull = CIRtmpPullFac::CreateRtmpPull(ZC_RTMPPULL_AIO_E);
    // m_pull = CIRtmpPullFac::CreateRtmpPull(ZC_RTMPPULL_E);
    if (!m_pull) {
        LOG_TRACE("CreateRtmpPull error");
        return false;
    }

    if (sendSMgrGetInfo(ZC_SHMSTREAM_PULLC, chn, &m_mediainfo) < 0) {
        LOG_TRACE("sendSMgrGetInfo error");
        goto _err;
    }

    if (!m_pull->Init(&cbinfo, chn, url)) {
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

bool CRtmpPullMan::_unInit() {
    Stop();
    if (m_pull) {
        m_pull->UnInit();
        delete m_pull;
    }

    return true;
}

bool CRtmpPullMan::UnInit() {
    if (!m_init) {
        return false;
    }

    _unInit();

    m_init = false;
    return true;
}
bool CRtmpPullMan::Start() {
    if (m_running) {
        return false;
    }

    if (m_pull) {
        m_running = m_pull->StartCli();
    }

    return m_running;
}

bool CRtmpPullMan::Stop() {
    if (!m_running) {
        return false;
    }

    m_pull->StopCli();
    m_running = false;
    return true;
}
}  // namespace zc
