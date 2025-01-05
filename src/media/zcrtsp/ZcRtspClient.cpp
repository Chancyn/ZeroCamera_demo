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
#include "media/ZcMediaReceiverFac.hpp"
#include "zc_h26x_sps_parse.h"

// CRtpReceiver reference "rtp-receiver-test.h"

//#define UDP_MULTICAST_ADDR "239.0.0.2"
#define ZC_RTSP_CLI_BUF_SIZE (2 * 1024 * 1024)

extern "C" int rtsp_addr_is_multicast(const char *ip);
extern "C" int rtsp_client_options(rtsp_client_t *rtsp, const char *commands);
namespace zc {
CRtspClient::CRtspClient()
    : Thread("RtspCli"), m_init(false), m_running(0), m_pbuf(new char[ZC_RTSP_CLI_BUF_SIZE]), m_phandle(nullptr) {
    memset(&m_client, 0, sizeof(m_client));
    m_keepalive = 0;
}

CRtspClient::~CRtspClient() {
    UnInit();
    for (unsigned int i = 0; i < ZC_MEIDIA_NUM; i++) {
        ZC_SAFE_DELETE(m_pRtp[i]);
        ZC_SAFE_DELETE(m_mediarecv[i]);
    }
    ZC_SAFE_DELETEA(m_pbuf);
}

bool CRtspClient::Init(rtspcli_callback_info_t *cbinfo, int chn, const char *url, int transport) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }
    m_transport = transport;
    m_chn = chn;
    memcpy(&m_cbinfo, cbinfo, sizeof(rtspcli_callback_info_t));
    strncpy(m_url, url, sizeof(m_url));
    // memcpy(&m_info, &info, sizeof(zc_stream_info_t));
    m_init = true;
    return true;
}

bool CRtspClient::UnInit() {
    StopCli();
    m_init = false;
    return true;
}

int CRtspClient::onframe(void *ptr1, void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(ptr1);
    return pcli->_onframe(ptr2, encode, packet, bytes, time, flags);
}

int CRtspClient::_onframe(void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags) {
    // LOG_TRACE("encode:%d, time:%08u, flags:%08d, drop.", encode, time, flags);
    CMediaReceiver *precv = reinterpret_cast<CMediaReceiver *>(ptr2);
    return precv->RtpOnFrameIn(packet, bytes, time, flags);
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
        // 0.0.0.0 set recvfrom any
        if (sockpair_create("0.0.0.0", m_client.rtp[media], m_client.port[media]) != 0) {
            // TODO(zhoucc): ipv6
            LOG_ERROR("socket error, media:%d,port:%hu-%hu,rtp:%d-%d", media, m_client.port[media][0],
                      m_client.port[media][1], m_client.rtp[media][0], m_client.rtp[media][1]);
            ZC_ASSERT(0);
            return 0;
        }
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

void CRtspClient::onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes) {
    CRtspClient *pcli = reinterpret_cast<CRtspClient *>(param);
    return pcli->_onrtp(channel, data, bytes);
}

void CRtspClient::_onrtp(uint8_t channel, const void *data, uint16_t bytes) {
    // must rtp channel
    if (channel % 2) {
        // TODO(zhoucc): rtcp channel ,maybe recv rtcp data? maybe todo
        return;
    }
    ZC_ASSERT(channel <= ZC_MEIDIA_NUM);
    ZC_ASSERT(m_pRtp[channel / 2] != nullptr);
    m_pRtp[channel / 2]->RtpReceiverTcpInput(channel, data, bytes);

    if (++m_keepalive % 1000 == 0) {
        // rtsp_client_play(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), NULL, NULL);
        rtsp_client_options(reinterpret_cast<rtsp_client_t *>(m_client.rtsp), NULL);
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

static inline int findTrackIndex(unsigned int tracktype, const zc_stream_info_t &stinfo) {
    for (unsigned int i = 0; i < stinfo.tracknum; i++) {
        if (tracktype == stinfo.tracks[i].tracktype) {
            LOG_TRACE("find i:%u, trackno:%u, tracktype:%u", i, stinfo.tracks[i].trackno, stinfo.tracks[i].tracktype);
            return i;
        }
    }

    LOG_ERROR("notfind tracktype:%u", tracktype);
    return -1;
}

int CRtspClient::_onsetup(int timeout, int64_t duration) {
    uint64_t npt = 0;
    char ip[65];
    u_short rtspport;
    int ret = 0;
    CMediaReceiverFac fac;
    zc_stream_info_t stinfo = {0};
    // get streaminfo
    if (!m_cbinfo.GetInfoCb || m_cbinfo.GetInfoCb(m_cbinfo.MgrContext, m_chn, &stinfo) < 0) {
        LOG_ERROR("rtsp_client_play m_cbinfo.GetInfoCb error");
        return -1;
    }

    for (unsigned int i = 0; i < stinfo.tracknum; i++) {
        stinfo.tracks[i].enable = 0;
        LOG_TRACE("diable i:%u, trackno:%u, tracktype:%u", i, stinfo.tracks[i].trackno, stinfo.tracks[i].tracktype);
    }

    rtsp_client_t *rtsp = reinterpret_cast<rtsp_client_t *>(m_client.rtsp);
    ret = rtsp_client_play(rtsp, &npt, NULL);
    ZC_ASSERT(0 == ret);
    if (ret != 0) {
        LOG_ERROR("rtsp_client_play error ret[%d]", ret);
        return -1;
    }

    int media_count = rtsp_client_media_count(rtsp);
    media_count = media_count < ZC_MEIDIA_NUM ? media_count : ZC_MEIDIA_NUM;
    unsigned int tracktype = 0;
    unsigned int encode = 0;
    int code = 0;
    int trackidx = 0;

    for (int i = 0; i < media_count; i++) {
        int payload, port[2];
        const char *encoding;
        const struct rtsp_header_transport_t *transport;
        transport = rtsp_client_get_media_transport(rtsp, i);
        encoding = rtsp_client_get_media_encoding(rtsp, i);
        payload = rtsp_client_get_media_payload(rtsp, i);
        code = CRtpReceiver::Encodingtrans2Type(encoding, tracktype, encode);
        // find trackidx
        if (code >= 0 && ((trackidx = findTrackIndex(tracktype, stinfo)) >= 0)) {
            // update code
            stinfo.tracks[trackidx].encode = encode;
            stinfo.tracks[trackidx].mediacode = CRtpReceiver::transEncode2MediaCode(encode);
            stinfo.tracks[trackidx].enable = 1;  // enable track
            // create MediaReceiver
            m_mediarecv[i] = fac.CreateMediaReceiver(stinfo.tracks[trackidx]);
            m_npt[i] = npt;
            LOG_WARN("m_npt:%llu", m_npt[i]);
        }

        if (m_mediarecv[i] && m_mediarecv[i]->Init()) {
            m_pRtp[i] = new CRtpReceiver(m_client.onframe, this, m_mediarecv[i]);
        } else {
            // donot regiester recv onframe callback
            LOG_ERROR("error, don't recv frame, i:%d, encoding:%s", i, encoding);
            m_pRtp[i] = new CRtpReceiver(NULL, this, NULL);
        }

        if (!m_pRtp[i]) {
            LOG_ERROR("udp new CRtpReceiver error");
            continue;
        }

        if (RTSP_TRANSPORT_RTP_UDP == transport->transport) {
            // assert(RTSP_TRANSPORT_RTP_UDP == transport->transport); // udp only
            assert(0 == transport->multicast);  // unicast only
            assert(transport->rtp.u.client_port1 == m_client.port[i][0]);
            assert(transport->rtp.u.client_port2 == m_client.port[i][1]);

            port[0] = transport->rtp.u.server_port1;
            port[1] = transport->rtp.u.server_port2;

            if (*transport->source) {
                m_pRtp[i]->RtpReceiverUdpStart(m_client.rtp[i], transport->source, port, payload, encoding);
            } else {
                socket_getpeername(m_client.socket, ip, &rtspport);
                m_pRtp[i]->RtpReceiverUdpStart(m_client.rtp[i], ip, port, payload, encoding);
            }
        } else if (RTSP_TRANSPORT_RTP_TCP == transport->transport) {
            // assert(transport->rtp.u.client_port1 == transport->interleaved1);
            // assert(transport->rtp.u.client_port2 == transport->interleaved2);
            m_pRtp[i]->RtpReceiverTcpStart(transport->interleaved1, transport->interleaved2, payload, encoding);

        } else {
            ZC_ASSERT(0);  // TODO(zhoucc)
        }
    }

    // update streaminfo
    if (!m_cbinfo.SetInfoCb || m_cbinfo.SetInfoCb(m_cbinfo.MgrContext, m_chn, &stinfo) < 0) {
        LOG_ERROR("rtsp_client_play m_cbinfo.GetInfoCb  ret[%d]", ret);
        return 0;
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
        ZC_SAFE_DELETE(m_mediarecv[i]);
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
    // nptbegin: nptend scale 点播时间 播放速度
    LOG_WARN("onplay %p, count,:%u media:%d, seq:%u, time:%u", this, count, media, rtpinfo->seq, rtpinfo->time);

    for (unsigned int i = 0; i < ZC_MEIDIA_NUM; i++) {
      if (media == i && m_mediarecv[i]) {
            LOG_WARN("onplay %p, count,:%u seq:%u, time:%u, npt:%llu", this, count, rtpinfo->seq, rtpinfo->time, m_npt[i]);
            m_mediarecv[i]->SetRtpInfo_Rtptime(rtpinfo->seq, rtpinfo->time, m_npt[i]);
        }
   }

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
    LOG_WARN("process into");
    while (State() == Running /*&&  i < loopcnt*/) {
        if (_cliwork() < 0) {
            break;
        }
        system_sleep(1000);
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
