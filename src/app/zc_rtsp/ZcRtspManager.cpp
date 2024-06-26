// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcRtspManager.hpp"
#include "ZcRtspServer.hpp"
#include "ZcType.hpp"

namespace zc {
CRtspManager::CRtspManager() : m_init(false), m_running(0) {}

CRtspManager::~CRtspManager() {
    UnInit();
}

int CRtspManager::_getStreamInfoCb(unsigned int type, unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("get info type:%u, chn:%d", type, chn);
    // TODO(zhoucc): cb
    return sendSMgrGetInfo(type, chn, info);
}

int CRtspManager::getStreamInfoCb(void *ptr, unsigned int type, unsigned int chn, zc_stream_info_t *info) {
    CRtspManager *pRtsp = reinterpret_cast<CRtspManager *>(ptr);
    return pRtsp->_getStreamInfoCb(type, chn, info);
}

bool CRtspManager::Init(rtsp_callback_info_t *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }
    rtspsvr_cb_info_t stSvrCbinfo = {
        .getStreamInfoCb = CRtspManager::getStreamInfoCb,
        .MgrContext = this,
    };

    if (!CModRtsp::Init(cbinfo)) {
        LOG_TRACE("CModRtsp Init error");
        goto _err;
    }

    if (!CRtspServer::Init(&stSvrCbinfo)) {
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

#if 1  // ZC_DEBUG_DUMP
static inline void _dumpTrackInfo(const char *user, zc_meida_track_t *info) {
    LOG_TRACE("[%s] ch:%u,trackno:%u,track:%u,encode:%u,mediacode:%u,en:%u,size:%u,fmaxlen:%u,name:%s", user, info->chn,
              info->trackno, info->tracktype, info->encode, info->mediacode, info->enable, info->fifosize,
              info->framemaxlen, info->name);
    return;
}

static inline void _dumpStreamInfo(const char *user, zc_stream_info_t *info) {
    LOG_TRACE("[%s] type:%d,idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype, info->idx, info->chn,
              info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}
#endif
}  // namespace zc
