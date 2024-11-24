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
#include "zc_h26x_sps_parse.h"
#include "zc_log.h"
#include "zc_macros.h"
// #include "srt/srt.h"

#include "Thread.hpp"
#include "ZcSrtPull.hpp"
#include "ZcSrtCaller.hpp"
#include "ZcTsDemuxer.hpp"
#include "ZcType.hpp"

#if 1
#define ZC_SRT_CLI_BUF_SIZE (2 * 1024 * 1024)
namespace zc {
CSrtPull::CSrtPull()
    : Thread("srtpull"), m_init(false), m_running(0), m_pbuf(new char[ZC_SRT_CLI_BUF_SIZE]), m_mpegts(nullptr),
      m_streamsput(nullptr) {}

CSrtPull::~CSrtPull() {
    UnInit();
    ZC_SAFE_DELETEA(m_pbuf);
}

bool CSrtPull::Init(srtpull_callback_info_t *cbinfo, int chn, const char *url) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }

    strncpy(m_url, url, sizeof(m_url));
    memcpy(&m_cbinfo, &cbinfo, sizeof(srtpull_callback_info_t));
    m_chn = chn;
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
    zc_srtcaller_info_t cbinfo = {
        .onRead = onSrtPullTsPkgCb,
        .ctx = this,
    };

    m_caller = new CSrtCaller(cbinfo);
    if (m_caller == nullptr) {
        return false;
    }

    LOG_TRACE("Open m_url[%s]", m_url);
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

int CSrtPull::onFrameCb(void *ptr, zc_frame_t *framehdr, const uint8_t *data) {
    return reinterpret_cast<CSrtPull *>(ptr)->_onFrameCb(framehdr, data);
}

int CSrtPull::_onFrameCb(zc_frame_t *framehdr, const uint8_t *data) {
    LOG_TRACE("_onFrameCb type:%u, code:%u, seqno:%u, size:%u, pts:%u, keyflag:%d", framehdr->type,
    framehdr->video.encode,
              framehdr->seq, framehdr->size, framehdr->pts, framehdr->keyflag);
    return m_streamsput->PutFrame(framehdr, data);
}

int32_t CSrtPull::onSrtPullTsPkgCb(void *ptr, const uint8_t *data, uint32_t bytes) {
    return reinterpret_cast<CSrtPull *>(ptr)->_onSrtPullTsPkgCb(data, bytes);
}

int32_t CSrtPull::_onSrtPullTsPkgCb(const uint8_t *data, uint32_t bytes)
{
     ZC_ASSERT(m_mpegts != nullptr);
     return m_mpegts->Input(data, bytes);
}

bool CSrtPull::_startTsDemuxer() {
    zc_tsdemuxer_info_t info = {};
    info.onframe = onFrameCb;
    info.ctx = this;
    if (m_mpegts) {
        delete m_mpegts;
        m_mpegts = nullptr;
    }

    m_mpegts = new CTsDemuxer(info);
    if (!m_mpegts) {
        LOG_ERROR("new CFlvDemuxer error");
        goto _err;
    }

    LOG_TRACE("startFlvMuxer OK");
    return true;
_err:
    delete m_mpegts;
    m_mpegts = nullptr;

    LOG_ERROR("startFlvMuxer error");
    return false;
}

bool CSrtPull::_stopTsDemuxer() {
    if (m_mpegts) {
        delete m_mpegts;
        m_mpegts = nullptr;
        LOG_ERROR("_stopFlvDemuxer ok");
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

    if (!_startTsDemuxer()) {
        LOG_ERROR("_startMpegTsMuxer error");
        goto _err;
    }

    while (State() == Running) {
        ZC_MSLEEP(5);
    }

_err:
    _stopTsDemuxer();
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