// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// reference media-server:rtp-receiver-test.c
#include <assert.h>
#include <cerrno>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ZcType.hpp"
#include "aio-socket.h"
#include "rtcp-header.h"
#include "rtp-demuxer.h"
#include "rtp-payload.h"
#include "rtp-profile.h"
#include "rtp.h"
#include "sockutil.h"
#include "sys/pollfd.h"
#include "sys/system.h"  // system_clock
#include "sys/thread.h"
#include "time64.h"

#include "ZcRtpReceiver.hpp"
#include "zc_log.h"
#include "zc_macros.h"

#define ZC_RTP_RECIVE_TIMEOUT 2000
namespace zc {

int CRtpRxThread::process() {
    LOG_WARN("process into\n");
    int ret = 0;
    while (State() == Running /*&&  i < loopcnt*/) {
        if ((ret = m_rtprx->RtpReceiver(ZC_RTP_RECIVE_TIMEOUT)) < 0) {
            LOG_WARN("RtpReceiver error ret[%d]\n", ret);
            ret = -1;
            break;
        }
        system_sleep(100);
    }
    LOG_WARN("process exit\n");
    return -1;
}

CRtpReceiver::CRtpReceiver() : m_running(RTP_STATUS_INIT), m_rtpctx(new rtp_context_t()), m_udpthread(nullptr) {}

CRtpReceiver::~CRtpReceiver() {
    RtpReceiverStop();
    ZC_SAFE_FREE(m_rtpctx);
}

int CRtpReceiver::_rtpRead(socket_t s) {
    int r;
    uint8_t size[2];
    static int i, n = 0;
    socklen_t len;
    struct sockaddr_storage ss;
    len = sizeof(ss);

    r = recvfrom(s, m_rtpctx->rtp_buffer, sizeof(m_rtpctx->rtp_buffer), 0, (struct sockaddr *)&ss, &len);
    if (r < 12)
        return -1;
    assert(0 == socket_addr_compare((const struct sockaddr *)&ss, (const struct sockaddr *)&m_rtpctx->ss[0]));

    n += r;
    if (0 == i++ % 100)
        LOG_TRACE("packet: %d, seq: %u, size: %d/%d", i,
                  ((uint8_t)m_rtpctx->rtp_buffer[2] << 8) | (uint8_t)m_rtpctx->rtp_buffer[3], r, n);

    size[0] = r >> 8;
    size[1] = r >> 0;
    fwrite(size, 1, sizeof(size), m_rtpctx->frtp);
    fwrite(m_rtpctx->rtp_buffer, 1, r, m_rtpctx->frtp);

    r = rtp_demuxer_input(m_rtpctx->demuxer, m_rtpctx->rtp_buffer, r);
    return r;
}

int CRtpReceiver::_rtcpRead(socket_t s) {
    int r;
    socklen_t len;
    struct sockaddr_storage ss;
    len = sizeof(ss);
    r = recvfrom(s, m_rtpctx->rtcp_buffer, sizeof(m_rtpctx->rtcp_buffer), 0, (struct sockaddr *)&ss, &len);
    if (r < 12)
        return -1;
    assert(0 == socket_addr_compare((const struct sockaddr *)&ss, (const struct sockaddr *)&m_rtpctx->ss[1]));

    r = rtp_demuxer_input(m_rtpctx->demuxer, m_rtpctx->rtcp_buffer, r);
    if (RTCP_BYE == r) {
        printf("finished\n");
    }
    fflush(m_rtpctx->fp);
    return r;
}

int CRtpReceiver::RtpReceiver(int timeout) {
    LOG_TRACE("RtpReceiver into\n");
    int i, r;
    //	int interval;
    time64_t clock;
    struct pollfd fds[2];

    for (i = 0; i < 2; i++) {
        fds[i].fd = m_rtpctx->socket[i];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    clock = time64_now();
    while (m_running == RTP_STATUS_RUNNING) {
        // RTCP report
        r = rtp_demuxer_rtcp(m_rtpctx->demuxer, m_rtpctx->rtcp_buffer, sizeof(m_rtpctx->rtcp_buffer));
        if (r > 0)
            r = socket_sendto(m_rtpctx->socket[1], m_rtpctx->rtcp_buffer, r, 0,
                              (const struct sockaddr *)&m_rtpctx->ss[1],
                              socket_addr_len((const struct sockaddr *)&m_rtpctx->ss[1]));

        r = poll(fds, 2, timeout);
        while (-1 == r && EINTR == errno)
            r = poll(fds, 2, timeout);
        if (0 == r) {
            continue;  // timeout
        } else if (r < 0) {
            LOG_TRACE("epoll error ret[%d]errno [%d]\n", r, errno);
            m_running = RTP_STATUS_ERR;
            break;
        } else {
            if (0 != fds[0].revents) {
                _rtpRead(m_rtpctx->socket[0]);
                fds[0].revents = 0;
            }

            if (0 != fds[1].revents) {
                _rtcpRead(m_rtpctx->socket[1]);
                fds[1].revents = 0;
            }
        }
    }
    LOG_TRACE("RtpReceiver exit ret[%d], m_running[%d]\n", r, m_running);
    return -1;
}

int CRtpReceiver::rtpOnpacket(void *param, const void *packet, int bytes, uint32_t timestamp, int flags) {
    CRtpReceiver *pcli = reinterpret_cast<CRtpReceiver *>(param);
    return pcli->_rtpOnpacket(packet, bytes, timestamp, flags);
}

int CRtpReceiver::_rtpOnpacket(const void *packet, int bytes, uint32_t timestamp, int flags) {
    const uint8_t start_code[] = {0, 0, 0, 1};
    if (0 == strcmp("H264", m_rtpctx->encoding) || 0 == strcmp("H265", m_rtpctx->encoding)) {
        fwrite(start_code, 1, 4, m_rtpctx->fp);
    } else if (0 == strcasecmp("mpeg4-generic", m_rtpctx->encoding)) {
        uint8_t adts[7];
        int len = bytes + 7;
        uint8_t profile = 2;
        uint8_t sampling_frequency_index = 4;
        uint8_t channel_configuration = 2;
        adts[0] = 0xFF; /* 12-syncword */
        adts[1] = 0xF0 /* 12-syncword */ | (0 << 3) /*1-ID*/ | (0x00 << 2) /*2-layer*/ | 0x01 /*1-protection_absent*/;
        adts[2] =
            ((profile - 1) << 6) | ((sampling_frequency_index & 0x0F) << 2) | ((channel_configuration >> 2) & 0x01);
        adts[3] = ((channel_configuration & 0x03) << 6) | ((len >> 11) & 0x03);
        /*0-original_copy*/ /*0-home*/ /*0-copyright_identification_bit*/ /*0-copyright_identification_start*/
        adts[4] = (uint8_t)(len >> 3);
        adts[5] = ((len & 0x07) << 5) | 0x1F;
        adts[6] = 0xFC | ((len / 1024) & 0x03);
        fwrite(adts, 1, sizeof(adts), m_rtpctx->fp);
    } else if (0 == strcmp("MP4A-LATM", m_rtpctx->encoding)) {
        // add ADTS header
    }
    fwrite(packet, 1, bytes, m_rtpctx->fp);
    (void)timestamp;
    (void)flags;

    if (0 == strcmp("H264", m_rtpctx->encoding)) {
        uint8_t type = *(uint8_t *)packet & 0x1f;
        if (0 < type && type <= 5) {
            // VCL frame
        }
    } else if (0 == strcmp("H265", m_rtpctx->encoding)) {
        uint8_t type = (*(uint8_t *)packet >> 1) & 0x3f;
        if (type <= 32) {
            // VCL frame
        }
    }

    return 0;
}

bool CRtpReceiver::RtpReceiverUdpStart(socket_t rtp[2], const char *peer, int peerport[2], int payload,
                                       const char *encoding) {
    if (!m_rtpctx)
        false;
    size_t n;
    pthread_t t;
    const struct rtp_profile_t *profile;

    snprintf(m_rtpctx->rtp_buffer, sizeof(m_rtpctx->rtp_buffer), "%s.%d.%d.%s", peer, peerport[0], payload, encoding);
    snprintf(m_rtpctx->rtcp_buffer, sizeof(m_rtpctx->rtcp_buffer), "%s.%d.%d.%s.rtp", peer, peerport[0], payload,
             encoding);
    m_rtpctx->fp = fopen(m_rtpctx->rtp_buffer, "wb");
    m_rtpctx->frtp = fopen(m_rtpctx->rtcp_buffer, "wb");

    socket_getrecvbuf(rtp[0], &n);
    socket_setrecvbuf(rtp[0], 512 * 1024);
    socket_getrecvbuf(rtp[0], &n);

    profile = rtp_profile_find(payload);
    m_rtpctx->demuxer =
        rtp_demuxer_create(100, profile ? profile->frequency : 90000, payload, encoding, rtpOnpacket, this);
    if (NULL == m_rtpctx->demuxer) {
        LOG_ERROR("create rtp_demuxer_create error");
        goto _err;  // ignore
    }
    assert(0 == socket_addr_from(&m_rtpctx->ss[0], NULL, peer, (u_short)peerport[0]));
    assert(0 == socket_addr_from(&m_rtpctx->ss[1], NULL, peer, (u_short)peerport[1]));
    // assert(0 == connect(rtp[0], (struct sockaddr*)&m_rtpctx->ss[0], len));
    // assert(0 == connect(rtp[1], (struct sockaddr*)&m_rtpctx->ss[1], len));

    snprintf(m_rtpctx->encoding, sizeof(m_rtpctx->encoding), "%s", encoding);
    m_rtpctx->socket[0] = rtp[0];
    m_rtpctx->socket[1] = rtp[1];
    m_running = RTP_STATUS_RUNNING;
    m_udpthread = new CRtpRxThread(this);
    if (!m_udpthread) {
        LOG_ERROR("create CRtpRxThread error");
        goto _err;
    }
    m_udpthread->Start();

    return true;
_err:
    m_running = RTP_STATUS_ERR;
    LOG_ERROR("start udp error");
    return false;
}

bool CRtpReceiver::RtpReceiverTcpStart(uint8_t interleave1, uint8_t interleave2, int payload, const char *encoding) {
    if (!m_rtpctx)
        return false;

    const struct rtp_profile_t *profile;

    snprintf(m_rtpctx->rtp_buffer, sizeof(m_rtpctx->rtp_buffer), "tcp.%d.%s", payload, encoding);
    snprintf(m_rtpctx->rtcp_buffer, sizeof(m_rtpctx->rtcp_buffer), "tcp.%d.%s.rtp", payload, encoding);
    m_rtpctx->fp = fopen(m_rtpctx->rtp_buffer, "wb");
    m_rtpctx->frtp = fopen(m_rtpctx->rtcp_buffer, "wb");
    snprintf(m_rtpctx->encoding, sizeof(m_rtpctx->encoding), "%s", encoding);

    profile = rtp_profile_find(payload);
    m_rtpctx->demuxer =
        rtp_demuxer_create(100, profile ? profile->frequency : 90000, payload, encoding, rtpOnpacket, this);
    if (!m_rtpctx->demuxer) {
        LOG_ERROR("rtp_demuxer_create error");
        goto _err;
    }

    m_running = RTP_STATUS_RUNNING;
    return true;
_err:
    m_running = RTP_STATUS_ERR;
    LOG_ERROR("start udp error");
    return false;
}

bool CRtpReceiver::RtpReceiverStop() {
    ZC_SAFE_DELETE(m_udpthread);

    if (m_rtpctx->demuxer) {
        rtp_demuxer_destroy(&m_rtpctx->demuxer);
        m_rtpctx->demuxer = nullptr;
    }

    if (m_rtpctx->frtp) {
        fclose(m_rtpctx->frtp);
        m_rtpctx->frtp = nullptr;
    }

    if (m_rtpctx->fp) {
        fclose(m_rtpctx->fp);
        m_rtpctx->fp = nullptr;
    }

    return true;
}

int CRtpReceiver::RtpReceiverTcpInput(uint8_t channel, const void *data, uint16_t bytes) {
    int r;
    uint8_t size[2];
    if (0 == channel % 2) {
        size[0] = bytes >> 8;
        size[1] = bytes >> 0;
        fwrite(size, 1, sizeof(size), m_rtpctx->frtp);
        fwrite(data, 1, bytes, m_rtpctx->frtp);
    }

    if (m_rtpctx->demuxer) {
        r = rtp_demuxer_input(m_rtpctx->demuxer, data, bytes);
        ZC_ASSERT(r >= 0);
        if (r < 0) {
            LOG_ERROR("rtp_demuxer_input error r[%]", r);
            return -1;
        }
    }

    return 0;
}

}  // namespace zc