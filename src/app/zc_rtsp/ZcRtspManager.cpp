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

// RtspManager handle mod msg callback
int CRtspManager::RtspMgrHandleMsg(void *ptr, unsigned int type, void *indata, void *outdata) {
    CRtspManager *pRtsp = reinterpret_cast<CRtspManager *>(ptr);
    return pRtsp->_rtspMgrHandleMsg(type, indata, outdata);
}

// RtspManager handle mod msg callback
int CRtspManager::_rtspMgrHandleMsg(unsigned int type, void *indata, void *outdata) {
    // LOG_TRACE("RtspMgrCb ptr:%p, type:%d, indata:%d", ptr, type);
    if (type == 0) {
        // TODO(zhoucc):
    }

    return -1;
}

// RtspManager handle mod subscribe msg callback
int CRtspManager::RtspMgrHandleSubMsg(void *ptr, unsigned int type, void *indata) {
    CRtspManager *pRtsp = reinterpret_cast<CRtspManager *>(ptr);
    return pRtsp->_rtspMgrHandleSubMsg(type, indata);
}

int CRtspManager::_rtspMgrStreamUpdate(unsigned int type, unsigned int chn) {
    LOG_TRACE("_rtspMgrHandleSubMsg, type:%d", type);
    CRtspServer::RtspMgrStreamUpdate(type, chn);
    return 0;
}

int CRtspManager::_rtspMgrHandleSubMsg(unsigned int type, void *indata) {
    LOG_TRACE("_rtspMgrHandleSubMsg, type:%d", type);
    int ret = -1;
    switch (type) {
    case RTSP_MGR_HDL_SUB_STREAMUPDATE_E: {
        zc_mod_pub_streamupdate_t *info = reinterpret_cast<zc_mod_pub_streamupdate_t *>(indata);
        ret = _rtspMgrStreamUpdate(info->type, info->chn);
        break;
    }
    default:
        break;
    }

    return ret;
}
}  // namespace zc
