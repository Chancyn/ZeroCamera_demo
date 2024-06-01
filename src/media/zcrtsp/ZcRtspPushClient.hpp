// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <mutex>

#include "rtsp-client.h"
#include "sockutil.h"
#include "zc_frame.h"
#include "zc_type.h"
#include "rtp-tcp-transport.h"
#include "rtp-udp-transport.h"

#include "Thread.hpp"
#include "ZcRtpReceiver.hpp"

#define ZC_MEIDIA_NUM 8  // #define N_MEDIA 8 from rtsp-client-internal.h

namespace zc {
struct zc_rtsp_client_push_t {
    std::shared_ptr<IMediaSource> source;
    std::shared_ptr<IRTPTransport> transport[5];
    rtsp_client_t *rtsp;
    socket_t socket;
    char sdp[2 * 1024];
    char host[128];
    int transportmode;
    int status;
    socket_t rtp[5][2];
    unsigned short port[5][2];
};

typedef enum {
    ZC_RTSP_TRANSPORT_RTP_UDP = 1,
    ZC_RTSP_TRANSPORT_RTP_TCP,
    ZC_RTSP_TRANSPORT_RAW,
} zc_rtsp_transport_e;

class CRtspPushClient : protected Thread {
 public:
    explicit CRtspPushClient(const char *url, int chn = 0, int transport = ZC_RTSP_TRANSPORT_RTP_UDP);
    virtual ~CRtspPushClient();

 public:
    bool StartCli();
    bool StopCli();

 private:
    bool _startconn();
    bool _stopconn();
    virtual int process();
    int _cliwork();

    int rtsp_client_sdp(const char *host);
    static int rtp_send_interleaved_data(void *ptr, const void *data, size_t bytes);
    int _send_interleaved_data(const void *data, size_t bytes);
    static int rtsp_client_send(void *param, const char *uri, const void *req, size_t bytes);
    int _send(const char *uri, const void *req, size_t bytes);
    static int rtpport(void *param, int media, const char *source, unsigned short rtp[2], char *ip, int len);
    int _rtpport(int media, const char *source, unsigned short rtp[2], char *ip, int len);
    static void onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes);
    void _onrtp(uint8_t channel, const void *data, uint16_t bytes);
    static int onannounce(void *param);
    int _onannounce();
    static int onsetup(void *param, int timeout, int64_t duration);
    int _onsetup(int timeout, int64_t duration);
    static int onteardown(void *param);
    int _onteardown();
    static int onrecord(void *param, int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale,
                        const struct rtsp_rtp_info_t *rtpinfo, int count);
    int _onrecord(int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale,
                  const struct rtsp_rtp_info_t *rtpinfo, int count);

 private:
    bool m_init;
    int m_running;
    int m_chn;
    int m_transport;
    char *m_pbuf;  // buffer
    char m_host[128];
    char m_url[ZC_MAX_PATH];
    void *m_phandle;  // handle
    unsigned int m_keepalive;
    zc_rtsp_client_push_t m_client;

    std::mutex m_mutex;
};

}  // namespace zc
