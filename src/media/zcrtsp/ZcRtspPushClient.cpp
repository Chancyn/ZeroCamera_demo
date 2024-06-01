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
#include "ntp-time.h"
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
#include "ZcRtspPushClient.hpp"
#include "media/ZcLiveSource.hpp"
#include "zc_h26x_sps_parse.h"

//#define UDP_MULTICAST_ADDR "239.0.0.2"
#define ZC_RTSP_CLI_BUF_SIZE (2 * 1024 * 1024)

extern "C" int rtsp_addr_is_multicast(const char *ip);

namespace zc {
CRtspPushClient::CRtspPushClient(const char *url, int chn, int transport)
    : Thread("RtspCli"), m_init(false), m_running(0), m_chn(chn), m_transport(transport),
      m_pbuf(new char[ZC_RTSP_CLI_BUF_SIZE]), m_phandle(nullptr) {
    memset(&m_client, 0, sizeof(m_client));
    if (url)
        strncpy(m_url, url, sizeof(m_url));
    m_keepalive = 0;
}

CRtspPushClient::~CRtspPushClient() {
    StopCli();
    ZC_SAFE_DELETEA(m_pbuf);
}

int CRtspPushClient::rtsp_client_sdp(const char *host) {
    static const char *pattern_live = "v=0\n"
                                      "o=- %llu %llu IN IP4 %s\n"
                                      "s=rtsp-client-push-test\n"
                                      "c=IN IP4 %s\n"
                                      "t=0 0\n"
                                      "a=range:npt=now-\n"  // live
                                      "a=recvonly\n"
                                      "a=control:*\n";  // aggregate control

    int offset = 0;

    int64_t duration;
    m_client.source.reset(new CLiveSource(m_chn));

    offset = snprintf(m_client.sdp, sizeof(m_client.sdp), pattern_live, ntp64_now(), ntp64_now(), "127.0.0.1", host);
    assert(offset > 0 && offset + 1 < sizeof(m_client.sdp));

    std::string sdpmedia;
    m_client.source->GetSDPMedia(sdpmedia);
    int len = snprintf(m_client.sdp + offset, sizeof(m_client.sdp) - offset, "%s", sdpmedia.c_str());
    return offset + len;
}

int CRtspPushClient::rtp_send_interleaved_data(void *ptr, const void *data, size_t bytes) {
    CRtspPushClient *psvr = reinterpret_cast<CRtspPushClient *>(ptr);
    return psvr->_send_interleaved_data(data, bytes);
}

int CRtspPushClient::_send_interleaved_data(const void *data, size_t bytes) {
    // TODO(zhoucc): send multiple rtp packet once time;unuse
    // TODO(zhoucc): maybe needlock
    return bytes == socket_send(m_client.socket, data, bytes, 0) ? 0 : -1;
}

int CRtspPushClient::rtsp_client_send(void *param, const char *uri, const void *req, size_t bytes) {
    CRtspPushClient *pcli = reinterpret_cast<CRtspPushClient *>(param);
    return pcli->_send(uri, req, bytes);
}

int CRtspPushClient::_send(const char *uri, const void *req, size_t bytes) {
    // TODO: check uri and make socket
    // 1. uri != rtsp describe uri(user input)
    // 2. multi-uri if media_count > 1

    return socket_send_all_by_time(m_client.socket, req, bytes, 0, 2000);
}

int CRtspPushClient::rtpport(void *param, int media, const char *source, unsigned short rtp[2], char *ip, int len) {
    CRtspPushClient *pcli = reinterpret_cast<CRtspPushClient *>(param);
    return pcli->_rtpport(media, source, rtp, ip, len);
}

int CRtspPushClient::_rtpport(int media, const char *source, unsigned short rtp[2], char *ip, int len) {
    int m = rtsp_client_get_media_type(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), media);
    if (SDP_M_MEDIA_AUDIO != m && SDP_M_MEDIA_VIDEO != m)
        return 0;  // ignore

    switch (m_client.transportmode) {
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

    return m_client.transportmode;
}

int rtsp_client_options(rtsp_client_t *rtsp, const char *commands);
void CRtspPushClient::onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes) {
    CRtspPushClient *pcli = reinterpret_cast<CRtspPushClient *>(param);
    return pcli->_onrtp(channel, data, bytes);
}

void CRtspPushClient::_onrtp(uint8_t channel, const void *data, uint16_t bytes) {
    // do nothing, don't recv rtsp-push-server sendto pushcli rtpdata.
    // must rtp channel
    // if (channel % 2){
    //     // TODO(zhoucc): rtcp channel ,maybe recv rtcp data? maybe todo
    //     return;
    // }
    // ZC_ASSERT(channel <= ZC_MEIDIA_NUM);
    // ZC_ASSERT(m_pRtp[channel / 2] != nullptr);
    // m_pRtp[channel / 2]->RtpReceiverTcpInput(channel, data, bytes);

    // TODO(zhoucc): keepalive
    // if (++m_keepalive % 1000 == 0) {
    //     rtsp_client_play(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), NULL, NULL);
    // }
}

int CRtspPushClient::onannounce(void *param) {
    CRtspPushClient *pcli = reinterpret_cast<CRtspPushClient *>(param);
    return pcli->_onannounce();
}

int CRtspPushClient::_onannounce() {
    return rtsp_client_setup(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), m_client.sdp, strlen(m_client.sdp));
}

int CRtspPushClient::onsetup(void *param, int timeout, int64_t duration) {
    CRtspPushClient *pcli = reinterpret_cast<CRtspPushClient *>(param);
    return pcli->_onsetup(timeout, duration);
}

int CRtspPushClient::_onsetup(int timeout, int64_t duration) {
    int i;
    int ret = 0;
    uint64_t npt = 0;
    rtsp_client_t *rtsp = reinterpret_cast<rtsp_client_t *>(m_client.rtsp);
    int media_count = rtsp_client_media_count(rtsp);
    media_count = media_count < ZC_MEIDIA_NUM ? media_count : ZC_MEIDIA_NUM;

    for (int i = 0; i < media_count; i++) {
        int payload;
        unsigned short port[2];
        const char *encoding;
        const struct rtsp_header_transport_t *transport;
        char track[16] = {0};
        snprintf(track, sizeof(track) - 1, "track%d", i);
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
            if (m_client.source.get()) {
                m_client.transport[i] = std::make_shared<RTPUdpTransport>();
                assert(transport->rtp.u.server_port1 && transport->rtp.u.server_port2);
                strncpy(m_client.host, transport->destination, sizeof(m_client.host));
                const char *ip = m_client.host[0] ? m_client.host : m_host;

                ret = ((RTPUdpTransport *)m_client.transport[i].get())->Init(m_client.rtp[i], ip, port);
                if (ret != 0) {
                    LOG_ERROR("init transport error");
                    ZC_ASSERT(0);
                    continue;
                }
                m_client.source->SetTransport(track, m_client.transport[i]);
            }
        } else if (RTSP_TRANSPORT_RTP_TCP == transport->transport) {
            // assert(transport->rtp.u.client_port1 == transport->interleaved1);
            // assert(transport->rtp.u.client_port2 == transport->interleaved2);
            // TODO(zhoucc): todo TCP send data/client _send_interleaved_data
            if (m_client.source.get()) {
                m_client.transport[i] = std::make_shared<RTPTcpTransportCli>(
                    rtp_send_interleaved_data, this, transport->interleaved1, transport->interleaved2);
                ZC_ASSERT(m_client.transport[i].get() != nullptr);
                m_client.source->SetTransport(track, m_client.transport[i]);
            }
        } else {
            ZC_ASSERT(0);  // TODO(zhoucc)
        }
    }

    if (rtsp_client_record(m_client.rtsp, &npt, NULL) != 0) {
        LOG_ERROR("rtsp_client_record error");
        return -1;
    }
    return 0;
}

int CRtspPushClient::onteardown(void *param) {
    CRtspPushClient *pcli = reinterpret_cast<CRtspPushClient *>(param);
    return pcli->_onteardown();
}

int CRtspPushClient::_onteardown() {
    LOG_WARN("onteardown %p", this);
    // for (unsigned int i = 0; i < ZC_MEIDIA_NUM; i++) {
    //     ZC_SAFE_DELETE(m_pRtp[i]);
    // }

    return 0;
}

int CRtspPushClient::onrecord(void *param, int media, const uint64_t *nptbegin, const uint64_t *nptend,
                              const double *scale, const struct rtsp_rtp_info_t *rtpinfo, int count) {
    CRtspPushClient *pcli = reinterpret_cast<CRtspPushClient *>(param);
    return pcli->_onrecord(media, nptbegin, nptend, scale, rtpinfo, count);
}

int CRtspPushClient::_onrecord(int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale,
                               const struct rtsp_rtp_info_t *rtpinfo, int count) {
    m_client.status = 1;
    return 0;
}

bool CRtspPushClient::_startconn() {
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
    strncpy(m_host, host, sizeof(host)-1);
    if (url->userinfo) {
        userptr = strtok_r(url->userinfo, "@", &pswptr);
    }

    if (rtsp_client_sdp(host) <= 0) {
        LOG_ERROR("rtsppushcli error rtsp_client_sdp url[%s] this[%p]", m_url, this);
        goto _err;
    }
    LOG_DEBUG("parse url host[%s]port[%hu] path[%s] ok", host, url->port, path);
    LOG_DEBUG("parse url user[%s]psw[%s] ok", userptr, pswptr);
    phandle = (struct rtsp_client_handler_t *)malloc(sizeof(struct rtsp_client_handler_t));
    ZC_ASSERT(phandle);
    if (!phandle) {
        LOG_ERROR("rtsppushcli malloc error url[%s] this[%p]", m_url, this);
        goto _err;
    }

    m_phandle = phandle;
    phandle->send = rtsp_client_send;
    phandle->rtpport = rtpport;
    phandle->onannounce = onannounce;
    phandle->onsetup = onsetup;
    phandle->onrecord = onrecord;
    phandle->onteardown = onteardown;
    phandle->onrtp = onrtp;
    m_client.status = 0;
    if (ZC_RTSP_TRANSPORT_RTP_TCP == m_transport) {
        m_client.transportmode = RTSP_TRANSPORT_RTP_TCP;
    } else if (ZC_RTSP_TRANSPORT_RTP_UDP == m_transport) {
        m_client.transportmode = RTSP_TRANSPORT_RTP_UDP;
    } else if (ZC_RTSP_TRANSPORT_RAW == m_transport) {
        m_client.transportmode = RTSP_TRANSPORT_RAW;
    } else {
        m_client.transportmode = RTSP_TRANSPORT_RTP_TCP;
    }

    socket_init();
    m_client.socket = socket_connect_host(host, url->port, 2000);
    ZC_ASSERT(socket_invalid != m_client.socket);
    if (m_client.socket < 0) {
        LOG_ERROR("rtsppushcli connect error url[%s] this[%p]", m_url, this);
        goto _err;
    }

    // rtsp = rtsp_client_create(NULL, NULL, &handler, &ctx);
    rtsp = rtsp_client_create(m_url, userptr, pswptr, phandle, this);
    m_client.rtsp = rtsp;
    ZC_ASSERT(m_client.rtsp);
    if (!m_client.rtsp) {
        LOG_ERROR("rtsppushcli describe error url[%s] this[%p]", m_url, this);
        goto _err;
    }

    // pushcli send announce
    if (rtsp_client_announce(m_client.rtsp, m_client.sdp) != 0) {
        LOG_ERROR("rtsppushcli describe error url[%s] this[%p]", m_url, this);
        goto _err;
    }

    socket_setnonblock(m_client.socket, 0);
    LOG_WARN("rtsppushcli init ok url[%s] this[%p]", m_url, this);
    uri_free(url);
    return true;
_err:
    _stopconn();
    uri_free(url);
    LOG_ERROR("rtsppushcli _startconn error");
    return false;
}

bool CRtspPushClient::StartCli() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CRtspPushClient::StopCli() {
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

bool CRtspPushClient::_stopconn() {
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

int CRtspPushClient::_cliwork() {
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
                goto _err;
            }
            // connect success
            if (m_client.status == 1)
                break;
        }
    }

    while (State() == Running && m_client.status == 1) {
        system_sleep(5);

        if (1 == m_client.status)
            m_client.source->Play();

        // TODO: check rtsp session activity
    }

_err:

    rtsp_client_teardown(reinterpret_cast<rtsp_client_t *>(m_client.rtsp));
    // stop
    _stopconn();
    return ret;
}

int CRtspPushClient::process() {
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
