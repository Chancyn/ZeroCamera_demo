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
#include "ZcSrtPush.hpp"
#include "ZcTsMuxer.hpp"
#include "ZcType.hpp"
#include "ZcSrtCaller.hpp"

#if 1
#define ZC_SRT_CLI_BUF_SIZE (2 * 1024 * 1024)
#if 0
int client_testmain(int argc, char** argv)
{
    int ss, st;
    struct sockaddr_in sa;
    const int no = 0;
    const char message [] = "This message should be sent to the other side";

    if (argc != 3) {
      fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
      return 1;
    }

    printf("SRT startup\n");
    srt_startup();

    printf("Creating SRT socket\n");
    ss = srt_create_socket();
    if (ss == SRT_ERROR) {
        fprintf(stderr, "srt_socket: %s\n", srt_getlasterror_str());
        return 1;
    }

    printf("Creating remote address\n");
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &sa.sin_addr) != 1) {
        return 1;
    }

    int epollid = srt_epoll_create();
    if (epollid == -1)
    {
        fprintf(stderr, "srt_epoll_create: %s\n", srt_getlasterror_str());
        return 1;
    }

    printf("srt setsockflag\n");
    if (SRT_ERROR == srt_setsockflag(ss, SRTO_RCVSYN, &no, sizeof no)
        || SRT_ERROR == srt_setsockflag(ss, SRTO_SNDSYN, &no, sizeof no)) {
        fprintf(stderr, "SRTO_SNDSYN or SRTO_RCVSYN: %s\n", srt_getlasterror_str());
        return 1;
    }

    // When a caller is connected, a write-readiness event is triggered.
    int modes = SRT_EPOLL_OUT | SRT_EPOLL_ERR;
    if (SRT_ERROR == srt_epoll_add_usock(epollid, ss, &modes)) {
        fprintf(stderr, "srt_epoll_add_usock: %s\n", srt_getlasterror_str());
        return 1;
    }

    printf("srt connect\n");
    st = srt_connect(ss, (struct sockaddr*)&sa, sizeof sa);
    if (st == SRT_ERROR) {
        fprintf(stderr, "srt_connect: %s\n", srt_getlasterror_str());
        return 1;
    }

    // We had subscribed for write-readiness or error.
    // Write readiness comes in wready array,
    // error is notified via rready in this case.
    int       rlen = 1;
    SRTSOCKET rready;
    int       wlen = 1;
    SRTSOCKET wready;
    if (srt_epoll_wait(epollid, &rready, &rlen, &wready, &wlen, -1, 0, 0, 0, 0) != -1)
    {
        SRT_SOCKSTATUS state = srt_getsockstate(ss);
        if (state != SRTS_CONNECTED || rlen > 0) // rlen > 0 - an error notification
        {
            fprintf(stderr, "srt_epoll_wait: reject reason %s\n", srt_rejectreason_str(srt_getrejectreason(rready)));
            return 1;
        }

        if (wlen != 1 || wready != ss)
        {
            fprintf(stderr, "srt_epoll_wait: wlen %d, wready %d, socket %d\n", wlen, wready, ss);
            return 1;
        }
    } else {
        fprintf(stderr, "srt_connect: %s\n", srt_getlasterror_str());
        return 1;
    }

    int i;
    for (i = 0; i < 100; i++)
    {
        rready = SRT_INVALID_SOCK;
        rlen   = 1;
        wready = SRT_INVALID_SOCK;
        wlen   = 1;

        // As we have subscribed only for write-readiness or error events,
        // but have not subscribed for read-readiness,
        // through readfds we are notified about an error.
        int timeout_ms = 5000; // ms
        int res = srt_epoll_wait(epollid, &rready, &rlen, &wready, &wlen, timeout_ms, 0, 0, 0, 0);
        if (res == SRT_ERROR || rlen > 0)
        {
            fprintf(stderr, "srt_epoll_wait: %s\n", srt_getlasterror_str());
            return 1;
        }

        printf("srt sendmsg2 #%d >> %s\n", i, message);
        st = srt_sendmsg2(ss, message, sizeof message, NULL);
        if (st == SRT_ERROR)
        {
            fprintf(stderr, "srt_sendmsg: %s\n", srt_getlasterror_str());
            return 1;
        }

        usleep(1000);   // 1 ms
    }

    // Let's wait a bit so that all packets reach destination
    usleep(100000);   // 100 ms

    // In live mode the connection will be closed even if some packets were not yet acknowledged.
    printf("srt close\n");
    st = srt_close(ss);
    if (st == SRT_ERROR)
    {
        fprintf(stderr, "srt_close: %s\n", srt_getlasterror_str());
        return 1;
    }

    printf("srt cleanup\n");
    srt_cleanup();
    return 0;
}
#endif
namespace zc {
CSrtPush::CSrtPush()
    : Thread("srtpush"), m_init(false), m_running(0), m_pbuf(new char[ZC_SRT_CLI_BUF_SIZE]) {
}

CSrtPush::~CSrtPush() {
    UnInit();
    ZC_SAFE_DELETEA(m_pbuf);
}

bool CSrtPush::Init(const zc_stream_info_t &info, const char *url) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }

    strncpy(m_url, url, sizeof(m_url));
    memcpy(&m_info, &info, sizeof(zc_stream_info_t));
    m_init = true;
    return true;
}

bool CSrtPush::UnInit() {
    StopCli();
    m_init = false;
    return true;
}

// srt://127.0.0.1:10080?streamid=#!::h=srs.srt.com.cn,r=live/livestream,m=publish
bool CSrtPush::_startconn() {
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

bool CSrtPush::StartCli() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CSrtPush::StopCli() {
    LOG_TRACE("Stop into");
    if (m_running) {
        LOG_TRACE("socket_shutdown socket");
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CSrtPush::_stopconn() {
    if (m_caller) {
        m_caller->Close();
        ZC_SAFE_DELETE(m_caller);
    }

    return true;
}

int CSrtPush::onMpegTsPacketCb(void *ptr, const void *data, size_t bytes) {
    CSrtPush *pcli = reinterpret_cast<CSrtPush *>(ptr);
    return pcli->_onMpegTsPacketCb(data, bytes);
}

int CSrtPush::_onMpegTsPacketCb(const void *data, size_t bytes) {
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

bool CSrtPush::_startMpegTsMuxer() {
    m_mpegts = new CTsMuxer();
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

bool CSrtPush::_stopMpegTsMuxer() {
    if (m_mpegts) {
        m_mpegts->Stop();
        m_mpegts->Destroy();
        delete m_mpegts;
        m_mpegts = nullptr;
    }

    return true;
}

int CSrtPush::_cliwork() {
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

int CSrtPush::process() {
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