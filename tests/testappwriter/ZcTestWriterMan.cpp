// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>

#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"
#include "zc_stream_mgr.h"

#include "ZcTestWriterMan.hpp"
#include "ZcType.hpp"

#if ZC_LIVE_TEST
#include "ZcLiveTestWriterSys.hpp"
#endif

namespace zc {
// modsyscli
CTestWriterMan::CTestWriterMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0) {
    memset(&m_mediainfo[0], 0, sizeof(m_mediainfo));
}

CTestWriterMan::~CTestWriterMan() {
    UnInit();
}

int CTestWriterMan::_getStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli get info chn:%d", chn);
    // TODO(zhoucc): cb
    int ret = sendSMgrGetInfo(ZC_SHMSTREAM_LIVE, chn, info);
    if (ret < 0) {
        return ret;
    }
    memcpy(&m_mediainfo[chn], info, sizeof(zc_stream_info_t));
    return ret;
}

int CTestWriterMan::getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CTestWriterMan *pRtsp = reinterpret_cast<CTestWriterMan *>(ptr);
    return pRtsp->_getStreamInfoCb(chn, info);
}

int CTestWriterMan::_setStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli set info type:%u, chn:%d", chn);
    // TODO(zhoucc): cb
    if (memcmp(&m_mediainfo[chn], info, sizeof(zc_stream_info_t)) != 0) {
        memcpy(&m_mediainfo[chn], info, sizeof(zc_stream_info_t));
        return sendSMgrSetInfo(ZC_SHMSTREAM_LIVE, chn, &m_mediainfo[chn]);
    }

    return 0;
}

int CTestWriterMan::setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CTestWriterMan *pRtsp = reinterpret_cast<CTestWriterMan *>(ptr);
    return pRtsp->_setStreamInfoCb(chn, info);
}

bool CTestWriterMan::Init(unsigned int *pCodeTab, unsigned int len) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    testwriter_callback_info_t cbinfo = {
        .GetInfoCb = getStreamInfoCb,
        .SetInfoCb = setStreamInfoCb,
        .MgrContext = this,
    };

    for (unsigned int i = 0; i < ZC_STREAM_VIDEO_MAX_CHN; i++) {
        if (sendSMgrGetInfo(ZC_SHMSTREAM_LIVE, i, &m_mediainfo[i]) < 0) {
            LOG_TRACE("sendSMgrGetInfo error");
            goto _err;
        }
    }

#if ZC_LIVE_TEST
    g_ZCLiveTestWriterInstance.Init(cbinfo, pCodeTab, len);
#endif

    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    _unInit();

    LOG_TRACE("Init error");
    return false;
}  // namespace zc

bool CTestWriterMan::_unInit() {
    Stop();

    return true;
}

bool CTestWriterMan::UnInit() {
    if (!m_init) {
        return false;
    }

#if ZC_LIVE_TEST
    g_ZCLiveTestWriterInstance.UnInit();
#endif

    _unInit();

    m_init = false;
    return true;
}

bool CTestWriterMan::Start() {
    if (m_running) {
        return false;
    }

    m_running = true;
    return m_running;
}

bool CTestWriterMan::Stop() {
    if (!m_running) {
        return false;
    }
    m_running = false;
    return true;
}
}  // namespace zc
