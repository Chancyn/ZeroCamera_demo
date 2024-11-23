// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <mutex>

#include "aio-rtmp-client.h"
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
    aio_rtmp_client_t *rtmp;
    int status;
} zc_rtmp_pushaio_t;

class CRtmpPushAioCli : public CIRtmpPush, protected Thread {
 public:
    CRtmpPushAioCli();
    virtual ~CRtmpPushAioCli();

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
    static void aioOnConnect(void* ptr, int code, socket_t tcp, aio_socket_t aio);
    void _aioOnConnect(int code, socket_t tcp, aio_socket_t aio);
    static void rtmpClientPublishOnSend(void *ptr, size_t len);
    void _rtmpClientPublishOnSend(size_t len);
    static void rtmpClientPublishOnReady(void *ptr);
    void _rtmpClientPublishOnReady();
    static void rtmpClientPublishOnError(void* ptr, int code);
    void _rtmpClientPublishOnError(int code);
    static int onFlvPacketCb(void *ptr, int type, const void *data, size_t bytes, uint32_t timestamp);
    int _onFlvPacketCb(int type, const void *data, size_t bytes, uint32_t timestamp);
    virtual int process();

 private:
    bool m_init;
    int m_running;
    char m_url[ZC_MAX_PATH];
    zc_stream_info_t m_info;
    zc_rtmp_url_t m_rtmpurl;
    void *m_phandle;  // handle
    zc_rtmp_pushaio_t m_client;
    CFlvMuxer *m_flv;
    std::mutex m_mutex;
};

}  // namespace zc
