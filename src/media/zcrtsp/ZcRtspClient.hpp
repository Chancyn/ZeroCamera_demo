// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <mutex>

#include "media/ZcMediaReceiver.hpp"
#include "rtsp-client.h"
#include "sockutil.h"
#include "zc_frame.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcRtpReceiver.hpp"

#define ZC_MEIDIA_NUM 8  // #define N_MEDIA 8 from rtsp-client-internal.h

namespace zc {
// zhoucc mancallback
typedef int (*RtspCliGetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);
typedef int (*RtspCliSetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);

typedef struct {
    RtspCliGetInfoCb GetInfoCb;
    RtspCliSetInfoCb SetInfoCb;
    void *MgrContext;
} rtspcli_callback_info_t;

struct zc_rtsp_client_t {
    void *rtsp;
    rtponframe onframe;
    socket_t socket;

    int transport;
    socket_t rtp[5][2];
    unsigned short port[5][2];
};

typedef enum {
    ZC_RTSP_TRANSPORT_RTP_UDP = 1,
    ZC_RTSP_TRANSPORT_RTP_TCP,
    ZC_RTSP_TRANSPORT_RAW,
} zc_rtsp_transport_e;

class CRtspClient : protected Thread {
 public:
    CRtspClient();
    virtual ~CRtspClient();

 public:
    bool Init(rtspcli_callback_info_t *cbinfo, int chn, const char *url, int transport);
    bool UnInit();
    bool StartCli();
    bool StopCli();

 private:
    bool _startconn();
    bool _stopconn();
    virtual int process();
    int _cliwork();
#if 0
    inline int _frameH264(const void *packet, int bytes, uint32_t time, int flags);
    inline int _frameH265(const void *packet, int bytes, uint32_t time, int flags);
    inline int _frameAAC(const void *packet, int bytes, uint32_t time, int flags);
#endif

    // rtp on frame
    static int onframe(void *ptr1, void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags);
    int _onframe(void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags);

    static int send(void *param, const char *uri, const void *req, size_t bytes);
    int _send(const char *uri, const void *req, size_t bytes);
    static int rtpport(void *param, int media, const char *source, unsigned short rtp[2], char *ip, int len);
    int _rtpport(int media, const char *source, unsigned short rtp[2], char *ip, int len);
    static void onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes);
    void _onrtp(uint8_t channel, const void *data, uint16_t bytes);
    static int ondescribe(void *param, const char *sdp, int len);
    int _ondescribe(const char *sdp, int len);
    static int onsetup(void *param, int timeout, int64_t duration);
    int _onsetup(int timeout, int64_t duration);
    static int onteardown(void *param);
    int _onteardown();
    static int onplay(void *param, int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale,
                      const struct rtsp_rtp_info_t *rtpinfo, int count);
    int _onplay(int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale,
                const struct rtsp_rtp_info_t *rtpinfo, int count);
    static int onpause(void *param);
    int _onpause();

 private:
    bool m_init;
    int m_running;
    int m_chn;
    int m_transport;
    char *m_pbuf;  // buffer
    char m_url[ZC_MAX_PATH];
    void *m_phandle;  // handle
    unsigned int m_keepalive;
    zc_rtsp_client_t m_client;
    rtspcli_callback_info_t m_cbinfo;
    CRtpReceiver *m_pRtp[ZC_MEIDIA_NUM];  // [rtp&rtcp] Receiver
    CMediaReceiver *m_mediarecv[ZC_MEIDIA_NUM];
    uint64_t m_npt[ZC_MEIDIA_NUM];
    std::mutex m_mutex;
};

}  // namespace zc
