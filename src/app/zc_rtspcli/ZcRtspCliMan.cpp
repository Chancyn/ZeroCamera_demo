// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"
#include "zc_stream_mgr.h"

#include "ZcRtspCliMan.hpp"
#include "ZcType.hpp"

namespace zc {
// modsyscli
CRtspCliMan::CRtspCliMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0) {
    memset(&m_mediainfo, 0, sizeof(m_mediainfo));
}

CRtspCliMan::~CRtspCliMan() {
    UnInit();
}

int CRtspCliMan::_getStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli get info chn:%d", chn);
    // TODO(zhoucc): cb
    int ret = sendSMgrGetInfo(ZC_SHMSTREAM_PULLC, chn, info);
    if (ret < 0) {
        return ret;
    }
    memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
    return ret;
}

int CRtspCliMan::getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CRtspCliMan *pRtsp = reinterpret_cast<CRtspCliMan *>(ptr);
    return pRtsp->_getStreamInfoCb(chn, info);
}

int CRtspCliMan::_setStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli set info type:%u, chn:%d", chn);
    // TODO(zhoucc): cb
    if (memcmp(&m_mediainfo, info, sizeof(zc_stream_info_t)) != 0) {
        memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
        return sendSMgrSetInfo(ZC_SHMSTREAM_PULLC, chn, &m_mediainfo);
    }

    return 0;
}

int CRtspCliMan::setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CRtspCliMan *pRtsp = reinterpret_cast<CRtspCliMan *>(ptr);
    return pRtsp->_setStreamInfoCb(chn, info);
}

bool CRtspCliMan::Init(unsigned int chn, const char *url, int transport) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    rtspcli_callback_info_t cbinfo = {
        .GetInfoCb = getStreamInfoCb,
        .SetInfoCb = setStreamInfoCb,
        .MgrContext = this,
    };

    if (sendSMgrGetInfo(ZC_SHMSTREAM_PULLC, chn, &m_mediainfo) < 0) {
        LOG_TRACE("sendSMgrGetInfo error");
        goto _err;
    }

    if (!CRtspClient::Init(&cbinfo, chn, url, transport)) {
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

bool CRtspCliMan::_unInit() {
    Stop();
    CRtspClient::UnInit();

    return false;
}

bool CRtspCliMan::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}
bool CRtspCliMan::Start() {
    if (m_running) {
        return false;
    }

    m_running = CRtspClient::StartCli();
    return m_running;
}

bool CRtspCliMan::Stop() {
    if (!m_running) {
        return false;
    }

    CRtspClient::StopCli();
    m_running = false;
    return true;
}
}  // namespace zc
