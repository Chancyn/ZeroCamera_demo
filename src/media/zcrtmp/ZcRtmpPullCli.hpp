// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <mutex>

#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_rtmp_utils.h"
#include "aio-rtmp-client.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcFlvDemuxer.hpp"
#include "ZcIRtmp.hpp"

namespace zc {
typedef struct {
    aio_rtmp_client_t *rtmp;
    int status;
} zc_rtmp_pull_t;

class CRtmpPullCli : public CIRtmpPull, protected Thread {
 public:
    CRtmpPullCli();
    virtual ~CRtmpPullCli();

 public:
    bool Init(rtmppull_callback_info_t *cbinfo, int chn, const char *url);
    bool UnInit();
    bool StartCli();
    bool StopCli();

 private:
    static int rtmpClientPullOnVideo(void *ptr, const void *data, size_t bytes, uint32_t timestamp);
    int _rtmpClientPullOnVideo(const void *data, size_t bytes, uint32_t timestamp);
    static int rtmpClientPullOnAudio(void *ptr, const void *data, size_t bytes, uint32_t timestamp);
    int _rtmpClientPullOnAudio(const void *data, size_t bytes, uint32_t timestamp);
    static int rtmpClientPullOnScript(void *ptr, const void *data, size_t bytes, uint32_t timestamp);
    int _rtmpClientPullOnScript(const void *data, size_t bytes, uint32_t timestamp);
    static void rtmpClientPullOnError(void *ptr, int code);
    void _rtmpClientPullOnError(int code);

    static int onFrameCb(void *ptr, zc_flvframe_t *frame);
    int _onFrameCb(zc_flvframe_t *frame);
    static void aioOnConnect(void *ptr, int code, socket_t tcp, aio_socket_t aio);
    void _aioOnConnect(int code, socket_t tcp, aio_socket_t aio);
    bool _startFlvDemuxer();
    bool _stopFlvDemuxer();
    bool _startconn();
    bool _stopconn();
    virtual int process();
    int _cliwork();

 private:
    bool m_init;
    int m_running;
    int m_chn;
    char m_url[ZC_MAX_PATH];
    zc_rtmp_url_t m_rtmpurl;
    rtmppull_callback_info_t m_cbinfo;
    zc_stream_info_t m_info;
    zc_rtmp_pull_t m_client;
    void *m_phandle;  // handle
    CFlvDemuxer *m_flvmuxer;
    std::mutex m_mutex;
};

}  // namespace zc
