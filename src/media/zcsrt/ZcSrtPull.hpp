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

#pragma once
#include <stdint.h>

#include <mutex>
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcTsDemuxer.hpp"
#include "ZcSrtCaller.hpp"

namespace zc {
class CSrtCaller;
#if 1
class CSrtPull : protected Thread {
 public:
    CSrtPull();
    virtual ~CSrtPull();

 public:
    bool Init(const zc_stream_info_t &info, const char *url);
    bool UnInit();
    bool StartCli();
    bool StopCli();

 private:
    bool _startconn();
    bool _stopconn();
    bool _startMpegTsMuxer();
    bool _stopMpegTsMuxer();
    int _cliwork();

    static int onMpegTsPacketCb(void *ptr, const void *data, size_t bytes);
    int _onMpegTsPacketCb(const void *data, size_t bytes);
    virtual int process();

 private:
    bool m_init;
    int m_running;
    zc_stream_info_t m_info;
    CSrtCaller *m_caller;
    char *m_pbuf;  // buffer
    char m_host[128];
    char m_url[ZC_MAX_PATH];
    void *m_phandle;  // handle
    CTsDemuxer *m_mpegts;
    std::mutex m_mutex;
};
#endif
}  // namespace zc
