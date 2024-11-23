// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/*
 * SRT - Secure, Reliable, Transport
 * Copyright (c) 2021 Haivision Systems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; If not, see <http://www.gnu.org/licenses/>
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_h26x_sps_parse.h"
// #include "srt/srt.h"

#include "Thread.hpp"
#include "ZcSrtPull.hpp"
#include "ZcType.hpp"

#if 1
#define ZC_SRT_CLI_BUF_SIZE (2 * 1024 * 1024)
namespace zc {
CSrtPull::CSrtPull()
    : Thread("srtpull"), m_init(false), m_running(0), m_pbuf(new char[ZC_SRT_CLI_BUF_SIZE]) {
}

CSrtPull::~CSrtPull() {
    UnInit();
    ZC_SAFE_DELETEA(m_pbuf);
}

bool CSrtPull::Init(const zc_stream_info_t &info, const char *url) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }

    strncpy(m_url, url, sizeof(m_url));
    memcpy(&m_info, &info, sizeof(zc_stream_info_t));
    m_init = true;
    return true;
}

bool CSrtPull::UnInit() {
    StopCli();
    m_init = false;
    return true;
}

// srt://127.0.0.1:10080?streamid=#!::h=srs.srt.com.cn,r=live/livestream,m=publish
bool CSrtPull::_startconn() {
    int ret = 0;
    m_caller = new CSrtCaller();
    if (m_caller == nullptr) {
        return false;
    }
    ret = m_caller->Open(m_url, 0);
    if (ret < 0) {
        goto _err;
    }

    LOG_TRACE("srtpush _startconn ok");
    return true;
_err:
    _stopconn();
    LOG_ERROR("srtpush _startconn error");
    return false;
}

bool CSrtPull::StartCli() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CSrtPull::StopCli() {
    LOG_TRACE("Stop into");
    if (m_running) {
        LOG_TRACE("socket_shutdown socket");
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CSrtPull::_stopconn() {
    if (m_caller) {
        m_caller->Close();
        ZC_SAFE_DELETE(m_caller);
    }

    return true;
}

int CSrtPull::onMpegTsPacketCb(void *ptr, const void *data, size_t bytes) {
    CSrtPull *pcli = reinterpret_cast<CSrtPull *>(ptr);
    return pcli->_onMpegTsPacketCb(data, bytes);
}

int CSrtPull::_onMpegTsPacketCb(const void *data, size_t bytes) {
    if (!m_caller) {
        return 0;
    }
    // LOG_TRACE("ts onpkg, size:%d", bytes);
    int ret = 0;
    ret = m_caller->Write((const char *)data, bytes);
    if (ret != bytes) {
        LOG_ERROR("write error ts onpkg, size:%d", bytes);
    }

    return ret;
}

bool CSrtPull::_startMpegTsMuxer() {
    m_mpegts = new CTsDemuxer();
    if (!m_mpegts) {
        return false;
    }
    zc_tsmuxer_info_t info = {};
    info.onTsPacketCb = onMpegTsPacketCb;
    info.Context = this;
    memcpy(&info.streaminfo, &m_info, sizeof(zc_stream_info_t));

    if (!m_mpegts->Create(info)) {
        LOG_ERROR("TsMuxer create error");
        goto _err;
    }

    if (!m_mpegts->Start()) {
        LOG_ERROR("TsMuxer start error");
        goto _err;
    }

    LOG_TRACE("startMpegTsMuxer OK");
    return true;
_err:
    m_mpegts->Destroy();
    delete m_mpegts;
    m_mpegts = nullptr;

    LOG_ERROR("startMpegTsMuxer error");
    return false;
}

bool CSrtPull::_stopMpegTsMuxer() {
    if (m_mpegts) {
        m_mpegts->Stop();
        m_mpegts->Destroy();
        delete m_mpegts;
        m_mpegts = nullptr;
    }

    return true;
}

int CSrtPull::_cliwork() {
    // start
    int ret = 0;
    if (!_startconn()) {
        LOG_ERROR("_startconn error");
        return -1;
    }

    if (!_startMpegTsMuxer()) {
        LOG_ERROR("_startMpegTsMuxer error");
        goto _err;
    }

    while (State() == Running) {
        ZC_MSLEEP(5);
    }

_err:
    _stopMpegTsMuxer();
    // stop
    _stopconn();
    return ret;
}

int CSrtPull::process() {
    LOG_WARN("process into");
    while (State() == Running /*&&  i < loopcnt*/) {
        if (_cliwork() < 0) {
            break;
        }
        ZC_MSLEEP(1000);
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
#endif