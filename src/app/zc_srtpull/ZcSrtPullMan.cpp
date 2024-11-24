// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>

#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcSrtPullMan.hpp"
#include "ZcType.hpp"

namespace zc {
// modsyscli
CSrtPullMan::CSrtPullMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0), m_pull(nullptr) {
    memset(&m_mediainfo, 0, sizeof(zc_stream_info_t));
}

CSrtPullMan::~CSrtPullMan() {
    UnInit();
}

int CSrtPullMan::_getStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("srtpull get info chn:%d", chn);
    // TODO(zhoucc): cb
    int ret = sendSMgrGetInfo(ZC_SHMSTREAM_PULLC, chn, info);
    if (ret < 0) {
        return ret;
    }
    memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
    return ret;
}

int CSrtPullMan::getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    return reinterpret_cast<CSrtPullMan *>(ptr)->_getStreamInfoCb(chn, info);
}

int CSrtPullMan::_setStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("srtpull set info type:%u, chn:%d", chn);
    // TODO(zhoucc): cb
    if (memcmp(&m_mediainfo, info, sizeof(zc_stream_info_t)) != 0) {
        memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
        return sendSMgrSetInfo(ZC_SHMSTREAM_PULLC, chn, &m_mediainfo);
    }

    return 0;
}

int CSrtPullMan::setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    return  reinterpret_cast<CSrtPullMan *>(ptr)->_setStreamInfoCb(chn, info);
}

bool CSrtPullMan::Init(unsigned int chn, const char *url) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    srtpull_callback_info_t cbinfo = {
        .GetInfoCb = getStreamInfoCb,
        .SetInfoCb = setStreamInfoCb,
        .MgrContext = this,
    };

    // m_pull = CSrtPullFac::CreateSrtPull(ZC_RTMPPULL_AIO_E);
    m_pull = new CSrtPull();
    if (!m_pull) {
        LOG_TRACE("CreateSrtPull error");
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

bool CSrtPullMan::_unInit() {
    Stop();
    if (m_pull) {
        m_pull->UnInit();
        delete m_pull;
    }

    return true;
}

bool CSrtPullMan::UnInit() {
    if (!m_init) {
        return false;
    }

    _unInit();

    m_init = false;
    return true;
}
bool CSrtPullMan::Start() {
    if (m_running) {
        return false;
    }

    if (m_pull) {
        m_running = m_pull->StartCli();
    }

    return m_running;
}

bool CSrtPullMan::Stop() {
    if (!m_running) {
        return false;
    }

    m_pull->StopCli();
    m_running = false;
    return true;
}
}  // namespace zc
