// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// reference media-server:rtp-receiver-test.c

#pragma once
#include "NonCopyable.hpp"
#include "Thread.hpp"
#include "rtsp-client.h"
#include "sockutil.h"
#include "zc_type.h"
#if ZC_DEBUG
#define ZC_DEBUG_SAVE_RTP 1
#endif

typedef int (*rtponframe)(void *ptr1, void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags);

class CRtspClient;
namespace zc {
typedef struct _rtp_context_ {
#ifdef ZC_DEBUG_SAVE_RTP
    FILE *fp;    // debug for save rtp stream
    FILE *frtp;  // debug for save rtp stream
#endif

    char encoding[64];
    int encodetype;     // zc_frame_enc_e
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

class CRtpReceiver : public NonCopyable {
    enum {
        RTP_STATUS_ERR = -1,
        RTP_STATUS_INIT = 0,
        RTP_STATUS_RUNNING,
        RTP_STATUS_STOP,
    };

 public:
    // for rtsp-client, m_ptr1 = CRtspClient, ptr2 not use
    // for rtsp-push-server, m_ptr1 = CRtspServer, ptr2 = Session,
    CRtpReceiver(rtponframe onframe, void *ptr1, void *ptr2);
    virtual ~CRtpReceiver();

    static int Encodingtrans2Type(const char *encoding);
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
    int m_running;  // RTP_STATUS_ERR
    rtp_context_t *m_rtpctx;
    rtponframe m_onframe;
    void *m_ptr1;
    void *m_ptr2;
    CRtpRxThread *m_udpthread;  // udp receiver thread
};
}  // namespace zc