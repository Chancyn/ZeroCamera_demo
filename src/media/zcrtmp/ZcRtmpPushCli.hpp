// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <mutex>

#include "rtmp-client.h"
#include "sockutil.h"
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"
#include "zc_rtmp_utils.h"

#include "Thread.hpp"
#include "ZcFlvMuxer.hpp"
#include "ZcIRtmp.hpp"

#define ZC_MEIDIA_NUM 8  // #define N_MEDIA 8 from rtsp-client-internal.h

namespace zc {
typedef struct {
    socket_t socket;
    rtmp_client_t *rtmp;
    int status;
} zc_rtmp_push_t;

class CRtmpPushCli : public CIRtmpPush, protected Thread {
 public:
    CRtmpPushCli();
    virtual ~CRtmpPushCli();

 public:
    virtual bool Init(const zc_stream_info_t &info, const char *url);
    virtual bool UnInit();
    virtual bool StartCli();
    virtual bool StopCli();

 private:
    bool _startconn();
    bool _stopconn();
    bool _startFlvMuxer();
    bool _stopFlvMuxer();
    int _cliwork();
    static int rtmp_client_send(void *ptr, const void *header, size_t len, const void *data, size_t bytes);
    int _rtmp_client_send(const void *header, size_t len, const void *data, size_t bytes);

    static int onFlvPacketCb(void *ptr, int type, const void *data, size_t bytes, uint32_t timestamp);
    int _onFlvPacketCb(int type, const void *data, size_t bytes, uint32_t timestamp);
    virtual int process();

 private:
    bool m_init;
    int m_running;
    char m_url[ZC_MAX_PATH];
    zc_stream_info_t m_info;
    zc_rtmp_url_t m_rtmpurl;
    zc_rtmp_push_t m_client;
    CFlvMuxer *m_flv;
    std::mutex m_mutex;

    void *m_phandle;  // handle
    char *m_pbuf;    // buffer
};

}  // namespace zc
