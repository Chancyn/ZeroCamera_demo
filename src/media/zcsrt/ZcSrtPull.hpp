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
#include "ZcStreamsPut.hpp"

namespace zc {
class CSrtCaller;
// mancallback
typedef int (*SrtPullGetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);
typedef int (*SrtPullSetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);

typedef struct {
    SrtPullGetInfoCb GetInfoCb;
    SrtPullSetInfoCb SetInfoCb;
    void *MgrContext;
} srtpull_callback_info_t;

class CSrtPull : protected Thread {
 public:
    CSrtPull();
    virtual ~CSrtPull();

 public:
    bool Init(srtpull_callback_info_t *cbinfo, int chn, const char *url);
    bool UnInit();
    bool StartCli();
    bool StopCli();

 private:
    bool _startconn();
    bool _stopconn();
    bool _startTsDemuxer();
    bool _stopTsDemuxer();
    int _cliwork();
    static int onFrameCb(void *ptr, zc_frame_t *framehdr, const uint8_t *data);
    int _onFrameCb(zc_frame_t *framehdr, const uint8_t *data);
    static int32_t onSrtPullTsPkgCb(void *ptr, const uint8_t *data, uint32_t bytes);
    int32_t _onSrtPullTsPkgCb(const uint8_t *data, uint32_t bytes);
    virtual int process();

 private:
    bool m_init;
    int m_running;
    int m_chn;
    srtpull_callback_info_t m_cbinfo;
    CSrtCaller *m_caller;
    char *m_pbuf;  // buffer
    char m_host[128];
    char m_url[ZC_MAX_PATH];
    void *m_phandle;  // handle
    CTsDemuxer *m_mpegts;
    CStreamsPut *m_streamsput;
    std::mutex m_mutex;
};

}  // namespace zc
