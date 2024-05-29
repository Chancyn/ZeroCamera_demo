// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// reference media-server:rtp-receiver-test.c

#pragma once
#include "rtsp-client.h"

#include "Thread.hpp"
#include "sockutil.h"
#include "zc_type.h"

#if ZC_DEBUG
#define ZC_DEBUG_SAVE_RTP 1
#endif

namespace zc {
typedef struct _rtp_context_ {
#ifdef ZC_DEBUG_SAVE_RTP
    FILE *fp;    // debug for save rtp stream
    FILE *frtp;  // debug for save rtp stream
#endif

    char encoding[64];
    socket_t socket[2];
    struct sockaddr_storage ss[2];

    char rtp_buffer[64 * 1024];
    char rtcp_buffer[32 * 1024];

    struct rtp_demuxer_t *demuxer;
} rtp_context_t;

class CRtpReceiver;
class CRtpRxThread : public zc::Thread {
 public:
    explicit CRtpRxThread(CRtpReceiver *rtprx) : Thread("RtpRx"), m_rtprx(rtprx) {}
    // CRtpRxThread(socket_t s) : Thread(std::string("RtpRx" + std::to_string(s))) {}
    virtual ~CRtpRxThread() {}
    virtual int process();

 private:
    CRtpReceiver *m_rtprx;
};

class CRtpReceiver {
    enum {
        RTP_STATUS_ERR = -1,
        RTP_STATUS_INIT = 0,
        RTP_STATUS_RUNNING,
        RTP_STATUS_STOP,
    };

 public:
    CRtpReceiver();
    virtual ~CRtpReceiver();

    int RtpReceiver(int timeout);
    bool RtpReceiverUdpStart(socket_t rtp[2], const char *peer, int peerport[2], int payload, const char *encoding);
    bool RtpReceiverTcpStart(uint8_t interleave1, uint8_t interleave2, int payload, const char *encoding);
    int RtpReceiverTcpInput(uint8_t channel, const void *data, uint16_t bytes);
    bool RtpReceiverStop();

 private:
    int _rtpRead(socket_t s);
    int _rtcpRead(socket_t s);

    static int rtpOnpacket(void *param, const void *packet, int bytes, uint32_t timestamp, int flags);
    int _rtpOnpacket(const void *packet, int bytes, uint32_t timestamp, int flags);

 private:
    rtp_context_t *m_rtpctx;
    CRtpRxThread *m_udpthread;  // udp receiver thread
    int m_running;      // RTP_STATUS_ERR
};
}  // namespace zc