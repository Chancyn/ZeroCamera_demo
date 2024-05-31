// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// client
#include <netdb.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/types.h>

#include "ZcType.hpp"
#include "rtp-tcp-transport.h"
#include "rtp-udp-transport.h"

#include "cpm/unuse.h"
#include "cstringext.h"
#include "rtsp-client.h"
#include "sdp.h"
#include "sockpair.h"
#include "sockutil.h"
#include "sys/system.h"
#include "uri-parse.h"
#include "urlcodec.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcRtpReceiver.hpp"
#include "ZcRtspClient.hpp"
#include "zc_h26x_sps_parse.h"

#define RTP_RECIEIVER_TEST 0  // 0 media-server demo for test; 1 CRtpReceiver
#if RTP_RECIEIVER_TEST
#include "rtp-receiver-test.h"
void rtp_receiver_tcp_input(uint8_t channel, const void *data, uint16_t bytes);
void rtp_receiver_test(socket_t rtp[2], const char *peer, int peerport[2], int payload, const char *encoding);
void *rtp_receiver_tcp_test(uint8_t interleave1, uint8_t interleave2, int payload, const char *encoding);
#endif

//#define UDP_MULTICAST_ADDR "239.0.0.2"
#define ZC_RTSP_CLI_BUF_SIZE (2 * 1024 * 1024)

extern "C" int rtsp_addr_is_multicast(const char *ip);

namespace zc {
CRtspClient::CRtspClient(const char *url, int transport)
    : Thread("RtspCli"), m_init(false), m_running(0), m_transport(transport), m_pbuf(new char[ZC_RTSP_CLI_BUF_SIZE]),
      m_phandle(nullptr) {
    memset(&m_client, 0, sizeof(m_client));
    if (url)
        strncpy(m_url, url, sizeof(m_url));
    m_videobufsize = ZC_STREAM_MAXFRAME_SIZE;
    m_audiobufsize = ZC_STREAM_MAXFRAME_SIZE_A;
    m_videoframe = (zc_frame_t *)calloc(1, m_videobufsize + sizeof(zc_frame_t));
    m_audioframe = (zc_frame_t *)calloc(1, m_audiobufsize + sizeof(zc_frame_t));
    ZC_ASSERT(m_videoframe != nullptr);
    ZC_ASSERT(m_audioframe != nullptr);

    m_videoframe->magic = ZC_FRAME_VIDEO_MAGIC;
    m_videoframe->type = ZC_STREAM_VIDEO;
    m_videoframe->magic = ZC_FRAME_AUDIO_MAGIC;
    m_videoframe->type = ZC_STREAM_AUDIO;
    m_videopkgcnt = 0;
    m_lasttime = 0;
}

CRtspClient::~CRtspClient() {
    StopCli();
    for (unsigned int i = 0; i < ZC_MEIDIA_NUM; i++) {
        ZC_SAFE_DELETE(m_pRtp[i]);
    }
    ZC_SAFE_DELETEA(m_pbuf);
    ZC_SAFE_FREE(m_audioframe);
    ZC_SAFE_FREE(m_videoframe);
}

inline int CRtspClient::_frameH264(const void *packet, int bytes, uint32_t time, int flags) {
    uint8_t type = *(uint8_t *)packet & 0x1f;
    struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC, &_ts);
    m_lasttime = _ts.tv_sec;

    ZC_ASSERT(m_videoframe != nullptr);

    // sps;
    if (type == H264_NAL_UNIT_TYPE_SPS) {
        zc_h26x_sps_info_t spsinfo = {0};
        if (zc_h264_sps_parse((const uint8_t *)packet, bytes, &spsinfo) == 0) {
            m_videoframe->video.width = spsinfo.width;
            m_videoframe->video.height = spsinfo.height;
        } else {
            LOG_ERROR("h264 prase sps error");
        }
    }

    if (m_videoframe->size + 4 + bytes <= m_videobufsize) {
        m_videoframe->data[m_videoframe->size + 0] = 00;
        m_videoframe->data[m_videoframe->size + 1] = 00;
        m_videoframe->data[m_videoframe->size + 2] = 00;
        m_videoframe->data[m_videoframe->size + 3] = 01;

        memcpy(m_videoframe->data + m_videoframe->size + 4, packet, bytes);
        m_videoframe->video.nalu[m_videopkgcnt] = bytes + 4;
        m_videoframe->size += bytes + 4;
        m_videopkgcnt++;
    } else {
        LOG_ERROR("pack error, reset time:%08u, bytes[%u], size[%u], pkgcnt[%u]", time, bytes, m_videoframe->size,
                  m_videopkgcnt);
        m_videoframe->size = 0;
        m_videopkgcnt = 0;
        return 0;
    }

    // I frame, P Frame
    if (type == H264_NAL_UNIT_TYPE_CODED_SLICE_IDR || type == H264_NAL_UNIT_TYPE_CODED_SLICE_NON_IDR) {
        m_videoframe->keyflag = (type == H264_NAL_UNIT_TYPE_CODED_SLICE_IDR) ? ZC_FRAME_IDR : 0;
        m_videoframe->seq = m_videoframe->seq + 1;
        m_videoframe->utc = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
        m_videoframe->pts = m_videoframe->utc;
        m_videoframe->video.encode = ZC_FRAME_ENC_H264;

        if (m_videoframe->keyflag)
            LOG_TRACE("H264,time:%08u,utc:%u,len:%u,type:%d,flags:%d,wh:%hu*%hu", time, m_videoframe->utc,
                      m_videoframe->size, type, flags, m_videoframe->video.width, m_videoframe->video.height);
        // putto fifo
        m_videoframe->size = 0;
        m_videopkgcnt = 0;
    }

    return 0;
}

inline int CRtspClient::_frameH265(const void *packet, int bytes, uint32_t time, int flags) {
    uint8_t type = (*(uint8_t *)packet >> 1) & 0x3f;
    struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC, &_ts);
    m_lasttime = _ts.tv_sec;
    if (type >= H265_NAL_UNIT_CODED_SLICE_TRAIL_N && type <= H265_NAL_UNIT_CODED_SLICE_RASL_R) {
        // TODO(zhoucc): B frame
    } else if (type >= H265_NAL_UNIT_CODED_SLICE_BLA_W_LP && type <= H265_NAL_UNIT_CODED_SLICE_CRA) {
        // I Frame 16-21,
        // TODO(zhoucc): I frame
    } else if (type >= H265_NAL_UNIT_VPS) {
        // !vcl;
        // LOG_WARN("%p => encoding:H265, time:%08u, flags:%d, bytes:%d", this, time, flags, bytes);
    }

    if (type == H265_NAL_UNIT_SPS) {
        zc_h26x_sps_info_t spsinfo = {0};
        if (zc_h265_sps_parse((const uint8_t *)packet, bytes, &spsinfo) == 0) {
            m_videoframe->video.width = spsinfo.width;
            m_videoframe->video.height = spsinfo.height;
        } else {
            LOG_ERROR("h265 prase sps error");
        }
    }

    if (m_videoframe->size + 4 + bytes <= m_videobufsize) {
        m_videoframe->data[m_videoframe->size + 0] = 00;
        m_videoframe->data[m_videoframe->size + 1] = 00;
        m_videoframe->data[m_videoframe->size + 2] = 00;
        m_videoframe->data[m_videoframe->size + 3] = 01;

        memcpy(m_videoframe->data + m_videoframe->size + 4, packet, bytes);
        m_videoframe->video.nalu[m_videopkgcnt] = bytes + 4;
        m_videoframe->size += bytes + 4;
        m_videopkgcnt++;
    } else {
        LOG_ERROR("pack error, reset time:%08u, bytes[%u], size[%u], pkgcnt[%u]", time, bytes, m_videoframe->size,
                  m_videopkgcnt);
        m_videoframe->size = 0;
        m_videopkgcnt = 0;
        return 0;
    }

    if (type >= 0 && type <= H265_NAL_UNIT_CODED_SLICE_CRA) {
        m_videoframe->keyflag = (type >= H265_NAL_UNIT_CODED_SLICE_BLA_W_LP) ? ZC_FRAME_IDR : 0;
        m_videoframe->seq = m_videoframe->seq + 1;
        m_videoframe->utc = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
        m_videoframe->pts = m_videoframe->utc;
        m_videoframe->video.encode = ZC_FRAME_ENC_H265;

        if (m_videoframe->keyflag)
            LOG_TRACE("H265,time:%08u,utc:%u,len:%u,type:%d,flags:%d,wh:%hu*%hu,pkgcnt:%d", time, m_videoframe->utc,
                      m_videoframe->size, type, flags, m_videoframe->video.width, m_videoframe->video.height,
                      m_videopkgcnt);

        m_videoframe->size = 0;
        m_videopkgcnt = 0;
    }

    return 0;
}

inline int CRtspClient::_frameAAC(const void *packet, int bytes, uint32_t time, int flags) {
    struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC, &_ts);
    if (bytes + 7 <= m_audiobufsize) {
        uint8_t ADTS[] = {0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0xFC};
        int audioSamprate = 32000;  //
        int audioChannel = 2;       //
        switch (audioSamprate) {
        case 16000:
            ADTS[2] = 0x60;
            break;
        case 32000:
            ADTS[2] = 0x54;
            break;
        case 44100:
            ADTS[2] = 0x50;
            break;
        case 48000:
            ADTS[2] = 0x4C;
            break;
        case 96000:
            ADTS[2] = 0x40;
            break;
        default:
            break;
        }
        ADTS[3] = (audioChannel == 2) ? 0x80 : 0x40;

        int len = bytes + 7;
        len <<= 5;    // 8bit * 2 - 11 = 5(headerSize 11bit)
        len |= 0x1F;  // 5 bit    1
        ADTS[4] = len >> 8;
        ADTS[5] = len & 0xFF;
        memcpy(m_audioframe->data, ADTS, sizeof(ADTS));

        m_audioframe->keyflag = 0;
        m_audioframe->seq = 0;
        m_audioframe->utc = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
        m_audioframe->pts = m_audioframe->utc;
        m_audioframe->size = bytes + 7;
        m_audioframe->audio.encode = ZC_FRAME_ENC_AAC;

        memcpy(m_audioframe->data + 7, packet, bytes);

#if ZC_DEBUG
        LOG_TRACE("AAC,time:%08u,utc:%u,len:%d,flags:%d", time, m_audioframe->size, m_audioframe->utc, flags);
#endif
    }

    return 0;
}

int CRtspClient::onframe(void *ptr1, void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(ptr1);
    return pcli->_onframe(ptr2, encode, packet, bytes, time, flags);
}

int CRtspClient::_onframe(void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags) {
    if (flags) {
        LOG_TRACE("encode:%d, time:%08u, flags:%08d, drop.", encode, time, flags);
    }

    if (ZC_FRAME_ENC_H264 == encode) {
        return _frameH264(packet, bytes, time, flags);
    } else if (ZC_FRAME_ENC_H265 == encode) {
        return _frameH265(packet, bytes, time, flags);
    } else if (ZC_FRAME_ENC_AAC == encode) {
        return _frameAAC(packet, bytes, time, flags);
    }

    return -1;
}

int CRtspClient::send(void *param, const char *uri, const void *req, size_t bytes) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_send(uri, req, bytes);
}

int CRtspClient::_send(const char *uri, const void *req, size_t bytes) {
    // TODO: check uri and make socket
    // 1. uri != rtsp describe uri(user input)
    // 2. multi-uri if media_count > 1

    return socket_send_all_by_time(m_client.socket, req, bytes, 0, 2000);
}

int CRtspClient::rtpport(void *param, int media, const char *source, unsigned short rtp[2], char *ip, int len) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_rtpport(media, source, rtp, ip, len);
}

int CRtspClient::_rtpport(int media, const char *source, unsigned short rtp[2], char *ip, int len) {
    int m = rtsp_client_get_media_type(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), media);
    if (SDP_M_MEDIA_AUDIO != m && SDP_M_MEDIA_VIDEO != m)
        return 0;  // ignore

    switch (m_client.transport) {
    case RTSP_TRANSPORT_RTP_UDP:
        // TODO: ipv6
        assert(0 == sockpair_create("0.0.0.0", m_client.rtp[media], m_client.port[media]));
        rtp[0] = m_client.port[media][0];
        rtp[1] = m_client.port[media][1];

        if (rtsp_addr_is_multicast(ip)) {
            if (0 != socket_udp_multicast(m_client.rtp[media][0], ip, source, 16) ||
                0 != socket_udp_multicast(m_client.rtp[media][1], ip, source, 16))
                return -1;
        }
#if defined(UDP_MULTICAST_ADDR)
        else {
            if (0 != socket_udp_multicast(m_client.rtp[media][0], UDP_MULTICAST_ADDR, source, 16) ||
                0 != socket_udp_multicast(m_client.rtp[media][1], UDP_MULTICAST_ADDR, source, 16))
                return -1;
            snprintf(ip, len, "%s", UDP_MULTICAST_ADDR);
        }
#endif
        break;

    case RTSP_TRANSPORT_RTP_TCP:
        rtp[0] = 2 * media;
        rtp[1] = 2 * media + 1;
        break;

    default:
        assert(0);
        return -1;
    }

    return m_client.transport;
}

int rtsp_client_options(rtsp_client_t *rtsp, const char *commands);
void CRtspClient::onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_onrtp(channel, data, bytes);
}

void CRtspClient::_onrtp(uint8_t channel, const void *data, uint16_t bytes) {
    static int keepalive = 0;
#if RTP_RECIEIVER_TEST
    rtp_receiver_tcp_input(channel, data, bytes);
#else
    // must rtp channel
    if (channel % 2){
        // TODO(zhoucc): rtcp channel ,maybe recv rtcp data? maybe todo
        return;
    }
    ZC_ASSERT(channel <= ZC_MEIDIA_NUM);
    ZC_ASSERT(m_pRtp[channel / 2] != nullptr);
    m_pRtp[channel / 2]->RtpReceiverTcpInput(channel, data, bytes);
#endif
    if (++keepalive % 1000 == 0) {
        rtsp_client_play(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), NULL, NULL);
    }
}

int CRtspClient::ondescribe(void *param, const char *sdp, int len) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_ondescribe(sdp, len);
}

int CRtspClient::_ondescribe(const char *sdp, int len) {
    return rtsp_client_setup(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), sdp, len);
}

int CRtspClient::onsetup(void *param, int timeout, int64_t duration) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_onsetup(timeout, duration);
}

int CRtspClient::_onsetup(int timeout, int64_t duration) {
    uint64_t npt = 0;
    char ip[65];
    u_short rtspport;
    int ret = 0;
    ret = rtsp_client_play(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), &npt, NULL);
    ZC_ASSERT(0 == ret);
    if (ret != 0) {
        LOG_ERROR("rtsp_client_play error ret[%d]", ret);
        return -1;
    }
    rtsp_client_t *rtsp = reinterpret_cast<rtsp_client_t *>(m_client.rtsp);
    int media_count = rtsp_client_media_count(rtsp);
    media_count = media_count < ZC_MEIDIA_NUM ? media_count : ZC_MEIDIA_NUM;

    for (int i = 0; i < media_count; i++) {
        int payload, port[2];
        const char *encoding;
        const struct rtsp_header_transport_t *transport;
        transport = rtsp_client_get_media_transport(rtsp, i);
        encoding = rtsp_client_get_media_encoding(rtsp, i);
        payload = rtsp_client_get_media_payload(rtsp, i);
        if (RTSP_TRANSPORT_RTP_UDP == transport->transport) {
            // assert(RTSP_TRANSPORT_RTP_UDP == transport->transport); // udp only
            assert(0 == transport->multicast);  // unicast only
            assert(transport->rtp.u.client_port1 == m_client.port[i][0]);
            assert(transport->rtp.u.client_port2 == m_client.port[i][1]);

            port[0] = transport->rtp.u.server_port1;
            port[1] = transport->rtp.u.server_port2;
            if (*transport->source) {
#if RTP_RECIEIVER_TEST
                rtp_receiver_test(m_client.rtp[i], transport->source, port, payload, encoding);
#else
                m_pRtp[i] = new CRtpReceiver(m_client.onframe, this, NULL);
                if (!m_pRtp[i]) {
                    LOG_ERROR("udp new CRtpReceiver error");
                    continue;
                }
                m_pRtp[i]->RtpReceiverUdpStart(m_client.rtp[i], transport->source, port, payload, encoding);
#endif
            } else {
                socket_getpeername(m_client.socket, ip, &rtspport);
#if RTP_RECIEIVER_TEST
                rtp_receiver_test(m_client.rtp[i], ip, port, payload, encoding);
#else
                m_pRtp[i] = new CRtpReceiver(m_client.onframe, this, NULL);
                if (!m_pRtp[i]) {
                    LOG_ERROR("udp new CRtpReceiver error");
                    continue;
                }
                m_pRtp[i]->RtpReceiverUdpStart(m_client.rtp[i], ip, port, payload, encoding);
#endif
            }
        } else if (RTSP_TRANSPORT_RTP_TCP == transport->transport) {
// assert(transport->rtp.u.client_port1 == transport->interleaved1);
// assert(transport->rtp.u.client_port2 == transport->interleaved2);
#if RTP_RECIEIVER_TEST
            rtp_receiver_tcp_test(transport->interleaved1, transport->interleaved2, payload, encoding);
#else
            m_pRtp[i] = new CRtpReceiver(m_client.onframe, this, NULL);
            if (!m_pRtp[i]) {
                LOG_ERROR("udp new CRtpReceiver error");
                continue;
            }
            m_pRtp[i]->RtpReceiverTcpStart(transport->interleaved1, transport->interleaved2, payload, encoding);
#endif
        } else {
            ZC_ASSERT(0);  // TODO(zhoucc)
        }
    }

    return 0;
}

int CRtspClient::onteardown(void *param) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_onteardown();
}

int CRtspClient::_onteardown() {
    LOG_WARN("onteardown %p", this);
    for (unsigned int i = 0; i < ZC_MEIDIA_NUM; i++) {
        ZC_SAFE_DELETE(m_pRtp[i]);
    }

    return 0;
}

int CRtspClient::onplay(void *param, int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale,
                        const struct rtsp_rtp_info_t *rtpinfo, int count) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_onplay(media, nptbegin, nptend, scale, rtpinfo, count);
}

int CRtspClient::_onplay(int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale,
                         const struct rtsp_rtp_info_t *rtpinfo, int count) {
    // TODO(zhoucc)
    LOG_WARN("onplay %p", this);
    return 0;
}

int CRtspClient::onpause(void *param) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_onpause();
}

int CRtspClient::_onpause() {
    // TODO(zhoucc)
    LOG_WARN("onpause %p", this);
    return 0;
}

bool CRtspClient::_startconn() {
    char tmp[256];
    char host[128] = {0};
    char path[128] = {0};
    char *userptr = nullptr;
    char *pswptr = nullptr;
    strncpy(tmp, m_url, sizeof(tmp) - 1);
    struct rtsp_client_handler_t *phandle = nullptr;
    rtsp_client_t *rtsp = nullptr;
    // parse url
    struct uri_t *url = uri_parse(tmp, strlen(tmp));
    if (!url)
        return -1;

    url_decode(url->path, strlen(url->path), path, sizeof(path));
    url_decode(url->host, strlen(url->host), host, sizeof(host));

    if (url->userinfo) {
        userptr = strtok_r(url->userinfo, "@", &pswptr);
    }

    LOG_DEBUG("parse url host[%s]port[%hu] path[%s] ok", host, url->port, path);
    LOG_DEBUG("parse url user[%s]psw[%s] ok", userptr, pswptr);
    phandle = (struct rtsp_client_handler_t *)malloc(sizeof(struct rtsp_client_handler_t));
    ZC_ASSERT(phandle);
    if (!phandle) {
        LOG_ERROR("rtspcli malloc error url[%s] this[%p]", m_url, this);
        goto _err;
    }
    m_phandle = phandle;

    phandle->send = send;
    phandle->rtpport = rtpport;
    phandle->ondescribe = ondescribe;
    phandle->onsetup = onsetup;
    phandle->onplay = onplay;
    phandle->onpause = onpause;
    phandle->onteardown = onteardown;
    phandle->onrtp = onrtp;
    m_client.onframe = onframe;
    if (ZC_RTSP_TRANSPORT_RTP_TCP == m_transport) {
        m_client.transport = RTSP_TRANSPORT_RTP_TCP;
    } else if (ZC_RTSP_TRANSPORT_RTP_UDP == m_transport) {
        m_client.transport = RTSP_TRANSPORT_RTP_UDP;
    } else if (ZC_RTSP_TRANSPORT_RAW == m_transport) {
        m_client.transport = RTSP_TRANSPORT_RAW;
    } else {
        m_client.transport = RTSP_TRANSPORT_RTP_TCP;
    }

    socket_init();
    m_client.socket = socket_connect_host(host, url->port, 2000);
    ZC_ASSERT(socket_invalid != m_client.socket);
    if (m_client.socket < 0) {
        LOG_ERROR("rtspcli connect error url[%s] this[%p]", m_url, this);
        goto _err;
    }

    // rtsp = rtsp_client_create(NULL, NULL, &handler, &ctx);
    rtsp = rtsp_client_create(m_url, userptr, pswptr, phandle, this);
    m_client.rtsp = rtsp;
    ZC_ASSERT(m_client.rtsp);
    if (!m_client.rtsp) {
        LOG_ERROR("rtspcli describe error url[%s] this[%p]", m_url, this);
        goto _err;
    }

    if (rtsp_client_describe(rtsp) != 0) {
        LOG_ERROR("rtspcli describe error url[%s] this[%p]", m_url, this);
        goto _err;
    }

    socket_setnonblock(m_client.socket, 0);
    LOG_WARN("rtspcli init ok url[%s] this[%p]", m_url, this);
    uri_free(url);
    return true;
_err:
    _stopconn();
    uri_free(url);
    LOG_ERROR("rtspcli _startconn error");
    return false;
}

bool CRtspClient::StartCli() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CRtspClient::StopCli() {
    LOG_TRACE("Stop into");
    if (m_running) {
        // first shutdown socket wakeup block recv thead
        if (m_client.socket > 0)
            socket_shutdown(m_client.socket, SHUT_RDWR);
        LOG_TRACE("socket_shutdown socket", m_client.socket);
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CRtspClient::_stopconn() {
    if (m_client.rtsp) {
        rtsp_client_destroy(reinterpret_cast<rtsp_client_t *>(m_client.rtsp));
        m_client.rtsp = nullptr;
    }

    if (m_client.socket > 0) {
        socket_close(m_client.socket);
        m_client.socket = 0;
        socket_cleanup();
    }

    ZC_SAFE_FREE(m_phandle);
    return true;
}

int CRtspClient::_cliwork() {
    // start
    int ret = 0;
    if (!_startconn()) {
        LOG_TRACE("_startconn error");
        return -1;
    }

    int rret = 0;
    while (State() == Running) {
        rret = socket_recv(m_client.socket, m_pbuf, ZC_RTSP_CLI_BUF_SIZE, 0);
        if (rret > 0) {
            if (rtsp_client_input(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), m_pbuf, rret) != 0) {
                LOG_ERROR("input error, break");
                ret = -1;
                break;
            }
        }
    }

    rtsp_client_teardown(reinterpret_cast<rtsp_client_t *>(m_client.rtsp));
    // stop
    _stopconn();
    return ret;
}

int CRtspClient::process() {
    LOG_WARN("process into\n");
    while (State() == Running /*&&  i < loopcnt*/) {
        if (_cliwork() < 0) {
            break;
        }
        system_sleep(1000);
    }
    LOG_WARN("process exit\n");
    return -1;
}

}  // namespace zc
