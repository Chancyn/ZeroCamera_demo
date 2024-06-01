#ifndef _rtp_tcp_transport_h_
#define _rtp_tcp_transport_h_

#include "media/media-source.h"
#include "rtsp-server-internal.h"

class RTPTcpTransport : public IRTPTransport {
 public:
    RTPTcpTransport(rtsp_server_t *rtsp, uint8_t rtp, uint8_t rtcp) : m_rtp(rtp), m_rtcp(rtcp), m_rtsp(rtsp) {}
    virtual ~RTPTcpTransport() {}

 public:
    virtual int Send(bool rtcp, const void *data, size_t bytes) {
        assert(bytes < (1 << 16));
        if (bytes >= (1 << 16))
            return -E2BIG;

        m_packet[0] = '$';
        m_packet[1] = rtcp ? m_rtcp : m_rtp;
        m_packet[2] = (bytes >> 8) & 0xFF;
        m_packet[3] = bytes & 0xff;
        // TODO(zhoucc) :memcpy----
        memcpy(m_packet + 4, data, bytes);
        int r = rtsp_server_send_interleaved_data(m_rtsp, m_packet, bytes + 4);
        return 0 == r ? bytes : r;
    }

 private:
    uint8_t m_rtp;
    uint8_t m_rtcp;
    rtsp_server_t *m_rtsp;
    uint8_t m_packet[4 + (1 << 16)];
};

// add zhoucc
#include "rtsp-client.h"
typedef int (*rtpsend)(void *ptr, const void *data, size_t bytes);
class RTPTcpTransportCli : public IRTPTransport {
 public:
    // reigster send callback,
    RTPTcpTransportCli(rtpsend sendcb, void *ptr, uint8_t rtp, uint8_t rtcp)
        : m_rtp(rtp), m_rtcp(rtcp), m_rtpsend(sendcb), m_ptr(ptr) {}
    virtual ~RTPTcpTransportCli() {}

 public:
    virtual int Send(bool rtcp, const void *data, size_t bytes) {
        assert(bytes < (1 << 16));
        if (bytes >= (1 << 16))
            return -E2BIG;

        m_packet[0] = '$';
        m_packet[1] = rtcp ? m_rtcp : m_rtp;
        m_packet[2] = (bytes >> 8) & 0xFF;
        m_packet[3] = bytes & 0xff;
        memcpy(m_packet + 4, data, bytes);
        if (m_rtpsend) {
            int r = m_rtpsend(m_ptr, m_packet, bytes + 4);
            return 0 == r ? bytes : r;
        }

        return 0;
    }

 private:
    uint8_t m_rtp;
    uint8_t m_rtcp;
    rtpsend m_rtpsend;
    void *m_ptr;
    uint8_t m_packet[4 + (1 << 16)];
};
#endif /* !_rtp_tcp_transport_h_ */
