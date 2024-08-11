// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <mutex>
#include "ZcTsMuxer.hpp"
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcTsMuxer.hpp"


namespace zc {
typedef struct {
    void *srt;
    int status;
} zc_srt_push_t;

class CSrtPush : protected Thread {
 public:
    CSrtPush();
    virtual ~CSrtPush();

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
    char *m_pbuf;  // buffer
    char m_host[128];
    char m_url[ZC_MAX_PATH];
    void *m_phandle;  // handle
    zc_srt_push_t m_client;
    CTsMuxer *m_mpegts;
    std::mutex m_mutex;
};

}  // namespace zc
