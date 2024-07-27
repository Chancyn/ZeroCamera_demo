// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// start media-server
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "zc_basic_stream.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "aio-socket.h"
#include "aio-worker.h"

#include "cstringext.h"  // strstartswith
#include "media/h264-file-source.h"
#include "media/h265-file-source.h"
#include "media/mp4-file-source.h"
#include "media/ps-file-source.h"
#include "ntp-time.h"
#include "path.h"
#include "rtp-tcp-transport.h"
#include "rtp-udp-transport.h"
#include "rtsp-server-aio.h"
#include "sys/system.h"  // system_clock
#include "uri-parse.h"
#include "urlcodec.h"
#include "zc_type.h"

#include "ZcRtspServer.hpp"
#include "ZcType.hpp"
#include "media/ZcLiveSource.hpp"
#include "media/ZcLiveTestWriterSys.hpp"
#include "media/ZcMediaTrack.hpp"

#include "zc_stream_mgr.h"

#define ZC_N_AIO_THREAD (4)  // aio thread num
#define ZC_TEST_SESSION 1    // test示例代码
#define ZC_SUPPORT_VOD 1     // video on Demand 点播

#if defined(_HAVE_FFMPEG_)
#include "media/ffmpeg-file-source.h"
#include "media/ffmpeg-live-source.h"
#endif

#define UDP_MULTICAST_ADDR "239.0.0.2"
#define UDP_MULTICAST_PORT 6000

// ffplay rtsp://127.0.0.1/vod/video/abc.mp4
// Windows --> d:\video\abc.mp4
// Linux   --> ./video/abc.mp4

#if ZC_SUPPORT_VOD
#if defined(OS_WINDOWS)
static const char *s_workdir = "d:\\";
#else
static const char *s_workdir = "./";
#endif
#endif

namespace zc {

int CRtspServer::rtsp_uri_parse(const char *uri, std::string &path) {
    char path1[256];
    struct uri_t *r = uri_parse(uri, strlen(uri));
    if (!r)
        return -1;

    url_decode(r->path, strlen(r->path), path1, sizeof(path1));
    path = path1;
    uri_free(r);
    return 0;
}

// zhoucc ptr=rtsp->param; ptr2=session,
int CRtspServer::rtsp_ondescribe(void *ptr, rtsp_server_t *rtsp, const char *uri) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_ondescribe(ptr, rtsp, uri);
}

int CRtspServer::_findLiveSourceInfo(const char *filename, zc_stream_info_t *info) {
    zc_shmstream_e type = ZC_SHMSTREAM_LIVE;
    unsigned int chn = 0;
    sscanf(filename, "%*[^.].ch%d", &chn);
    if (zc_prase_livestreampath(filename, &type) < 0) {
        return -1;
    }

    LOG_TRACE("%s, type:%d, chn:%d", filename, type, chn);
    if (m_cbinfo.getStreamInfoCb) {
        return m_cbinfo.getStreamInfoCb(m_cbinfo.MgrContext, type, chn, info);
    }

    return -1;
}

int CRtspServer::_ondescribe(void *ptr, rtsp_server_t *rtsp, const char *uri) {
    static const char *pattern_vod = "v=0\n"
                                     "o=- %llu %llu IN IP4 %s\n"
                                     "s=%s\n"
                                     "c=IN IP4 0.0.0.0\n"
                                     "t=0 0\n"
                                     "a=range:npt=0-%.1f\n"
                                     "a=recvonly\n"
                                     "a=control:*\n";  // aggregate control

    static const char *pattern_live = "v=0\n"
                                      "o=- %llu %llu IN IP4 %s\n"
                                      "s=%s\n"
                                      "c=IN IP4 0.0.0.0\n"
                                      "t=0 0\n"
                                      "a=range:npt=now-\n"  // live
                                      "a=recvonly\n"
                                      "a=control:*\n";  // aggregate control

    std::string filename;
    std::map<std::string, TFileDescription>::const_iterator it;

    rtsp_uri_parse(uri, filename);
    int vod = 0;
    if (strstartswith(filename.c_str(), "/live/")) {
        filename = filename.c_str() + 6;
    }
#if ZC_SUPPORT_VOD  // TODO(zhoucc)
    else if (strstartswith(filename.c_str(), "/vod/")) {
        filename = path::join(s_workdir, filename.c_str() + 5);
        vod = 1;
    }
#endif
    else {
        // assert(0);
        // return -1;
        return rtsp_server_reply_describe(rtsp, 404 /*Not Found*/, NULL);
    }

    char buffer[1024] = {0};
    {
        // AutoThreadLocker locker(s_locker);
        std::lock_guard<std::mutex> locker(m_mutex);
        it = m_describes.find(filename);
        if (it == m_describes.end()) {
            // unlock
            TFileDescription describe;
            std::shared_ptr<IMediaSource> source;
            if (vod == 0) {
                zc_stream_info_t info = {0};
                if (_findLiveSourceInfo(filename.c_str(), &info) < 0) {
                    LOG_ERROR("live %s, 404 Not Find", filename.c_str());
                    return rtsp_server_reply_describe(rtsp, 404 /*Not Found*/, NULL);
                }
                source.reset(new CLiveSource(info));
            } else {
                LOG_TRACE("vod %s", filename.c_str());
                if (strendswith(filename.c_str(), ".ps"))
                    source.reset(new PSFileSource(filename.c_str()));
                else if (strendswith(filename.c_str(), ".h264"))
                    source.reset(new H264FileSource(filename.c_str()));
                else if (strendswith(filename.c_str(), ".h265"))
                    source.reset(new H265FileSource(filename.c_str()));
                else {
#if defined(_HAVE_FFMPEG_)
                    source.reset(new FFFileSource(filename.c_str()));
#else
                    source.reset(new MP4FileSource(filename.c_str()));
#endif
                }
                source->GetDuration(describe.duration);
            }

            source->GetSDPMedia(describe.sdpmedia);

            // re-lock
            // TODO(zhoucc): donot insert,every time create describes again
            it = m_describes.insert(std::make_pair(filename, describe)).first;
        }
    }

    //
    if (vod) {
        // vod testfile /live/xxx.h264,/live/xxx.h265,
        snprintf(buffer, sizeof(buffer), pattern_vod, ntp64_now(), ntp64_now(), "0.0.0.0", uri,
                 it->second.duration / 1000.0);
    } else {
        // live/live.ch, live/push.ch, live/pull.ch,
        snprintf(buffer, sizeof(buffer), pattern_live, ntp64_now(), ntp64_now(), "0.0.0.0", uri);
    }

    std::string sdp{buffer};
    sdp = sdp.append(it->second.sdpmedia);

    LOG_TRACE("222 live %s, sdp[%s]", buffer, sdp.c_str());
    return rtsp_server_reply_describe(rtsp, 200, sdp.c_str());
}

int CRtspServer::rtsp_onsetup(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                              const struct rtsp_header_transport_t transports[], size_t num) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onsetup(ptr, rtsp, uri, session, transports, num);
}

int CRtspServer::_onsetup(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                          const struct rtsp_header_transport_t transports[], size_t num) {
    std::string filename;
    char rtsp_transport[128];
    const struct rtsp_header_transport_t *transport = NULL;

    rtsp_uri_parse(uri, filename);
    int vod = 0;
    if (strstartswith(filename.c_str(), "/live/")) {
        filename = filename.c_str() + 6;
    }
#if ZC_SUPPORT_VOD  // TODO(zhoucc)
    else if (strstartswith(filename.c_str(), "/vod/")) {
        filename = path::join(s_workdir, filename.c_str() + 5);
        vod = 1;
    }
#endif
    else {
        // assert(0);
        // return -1;
        return rtsp_server_reply_setup(rtsp, 404, NULL, NULL);
    }

    if ('\\' == *filename.rbegin() || '/' == *filename.rbegin())
        filename.erase(filename.end() - 1);
    const char *basename = path_basename(filename.c_str());
    if (NULL == strchr(basename, '.'))  // filter track1
        filename.erase(basename - filename.c_str() - 1, std::string::npos);

// TODO(zhoucc) find session
#if ZC_TEST_SESSION
    TSessions::iterator it;
    if (session) {
        std::lock_guard<std::mutex> locker(m_mutex);
        it = m_sessions.find(session);
        if (it == m_sessions.end()) {
            // 454 Session Not Found
            return rtsp_server_reply_setup(rtsp, 454, NULL, NULL);
        } else {
            // don't support aggregate control
            if (0) {
                // 459 Aggregate Operation Not Allowed
                return rtsp_server_reply_setup(rtsp, 459, NULL, NULL);
            }
        }
    } else {
        rtsp_media_t item;
        item.rtsp = rtsp;
        item.channel = 0;
        item.status = 0;

        if (vod == 0) {
            zc_stream_info_t info = {0};
            if (_findLiveSourceInfo(filename.c_str(), &info) < 0) {
                LOG_ERROR("live %s, 404 Not Find", filename.c_str());
                return rtsp_server_reply_setup(rtsp, 404 /*Not Found*/, NULL, NULL);
            }
            item.media.reset(new CLiveSource(info));
        } else {
            if (strendswith(filename.c_str(), ".ps"))
                item.media.reset(new PSFileSource(filename.c_str()));
            else if (strendswith(filename.c_str(), ".h264"))
                item.media.reset(new H264FileSource(filename.c_str()));
            else if (strendswith(filename.c_str(), ".h265"))
                item.media.reset(new H265FileSource(filename.c_str()));
            else {
#if defined(_HAVE_FFMPEG_)
                item.media.reset(new FFFileSource(filename.c_str()));
#else
                item.media.reset(new MP4FileSource(filename.c_str()));
#endif
            }
        }

        char rtspsession[32];
        snprintf(rtspsession, sizeof(rtspsession), "%p", item.media.get());

        std::lock_guard<std::mutex> locker(m_mutex);
        it = m_sessions.insert(std::make_pair(rtspsession, item)).first;
    }
#endif
    assert(NULL == transport);
    for (size_t i = 0; i < num && !transport; i++) {
        if (RTSP_TRANSPORT_RTP_UDP == transports[i].transport) {
            // RTP/AVP/UDP
            transport = &transports[i];
        } else if (RTSP_TRANSPORT_RTP_TCP == transports[i].transport) {
            // RTP/AVP/TCP
            // 10.12 Embedded (Interleaved) Binary Data (p40)
            transport = &transports[i];
        }
    }
    if (!transport) {
        // 461 Unsupported Transport
        return rtsp_server_reply_setup(rtsp, 461, NULL, NULL);
    }

#if ZC_TEST_SESSION
    // TODO(zhoucc) find session
    rtsp_media_t &item = it->second;
    if (RTSP_TRANSPORT_RTP_TCP == transport->transport) {
        // 10.12 Embedded (Interleaved) Binary Data (p40)
        int interleaved[2];
        if (transport->interleaved1 == transport->interleaved2) {
            interleaved[0] = item.channel++;
            interleaved[1] = item.channel++;
        } else {
            interleaved[0] = transport->interleaved1;
            interleaved[1] = transport->interleaved2;
        }

        item.transport = std::make_shared<RTPTcpTransport>(rtsp, interleaved[0], interleaved[1]);
        item.media->SetTransport(path_basename(uri), item.transport);

        // RTP/AVP/TCP;interleaved=0-1
        snprintf(rtsp_transport, sizeof(rtsp_transport), "RTP/AVP/TCP;interleaved=%d-%d", interleaved[0],
                 interleaved[1]);
    } else if (transport->multicast) {
        unsigned short port[2] = {transport->rtp.u.client_port1, transport->rtp.u.client_port2};
        char multicast[SOCKET_ADDRLEN];
        // RFC 2326 1.6 Overall Operation p12

        if (transport->destination[0]) {
            // Multicast, client chooses address
            snprintf(multicast, sizeof(multicast), "%s", transport->destination);
            port[0] = transport->rtp.m.port1;
            port[1] = transport->rtp.m.port2;
        } else {
            // Multicast, server chooses address
            snprintf(multicast, sizeof(multicast), "%s", UDP_MULTICAST_ADDR);
            port[0] = UDP_MULTICAST_PORT;
            port[1] = UDP_MULTICAST_PORT + 1;
        }

        item.transport = std::make_shared<RTPUdpTransport>();
        if (0 != ((RTPUdpTransport *)item.transport.get())->Init(multicast, port)) {
            // log

            // 500 Internal Server Error
            return rtsp_server_reply_setup(rtsp, 500, NULL, NULL);
        }
        item.media->SetTransport(path_basename(uri), item.transport);

        // Transport: RTP/AVP;multicast;destination=224.2.0.1;port=3456-3457;ttl=16
        snprintf(rtsp_transport, sizeof(rtsp_transport), "RTP/AVP;multicast;destination=%s;port=%hu-%hu;ttl=%d",
                 multicast, port[0], port[1], 16);

        // 461 Unsupported Transport
        // return rtsp_server_reply_setup(rtsp, 461, NULL, NULL);
    } else {
        // unicast
        item.transport = std::make_shared<RTPUdpTransport>();

        assert(transport->rtp.u.client_port1 && transport->rtp.u.client_port2);
        unsigned short port[2] = {transport->rtp.u.client_port1, transport->rtp.u.client_port2};
        const char *ip = transport->destination[0] ? transport->destination : rtsp_server_get_client(rtsp, NULL);
        LOG_TRACE("ip:%s:%hu", ip, port);
        if (0 != ((RTPUdpTransport *)item.transport.get())->Init(ip, port)) {
            // log

            // 500 Internal Server Error
            return rtsp_server_reply_setup(rtsp, 500, NULL, NULL);
        }
        item.media->SetTransport(path_basename(uri), item.transport);

        // RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257;destination=xxxx
        snprintf(rtsp_transport, sizeof(rtsp_transport), "RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu%s%s",
                 transport->rtp.u.client_port1, transport->rtp.u.client_port2, port[0], port[1],
                 transport->destination[0] ? ";destination=" : "",
                 transport->destination[0] ? transport->destination : "");
    }
#endif

    return rtsp_server_reply_setup(rtsp, 200, it->first.c_str(), rtsp_transport);
}

int CRtspServer::rtsp_onplay(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                             const double *scale) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onplay(ptr, rtsp, uri, session, npt, scale);
}

int CRtspServer::_onplay(void * /*ptr*/, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                         const double *scale) {
#if ZC_TEST_SESSION
    // TODO(zhoucc) find session
    std::shared_ptr<IMediaSource> source;
    TSessions::iterator it;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        it = m_sessions.find(session ? session : "");
        if (it == m_sessions.end()) {
            // 454 Session Not Found
            return rtsp_server_reply_play(rtsp, 454, NULL, NULL, NULL);
        } else {
            // uri with track
            if (0) {
                // 460 Only aggregate operation allowed
                return rtsp_server_reply_play(rtsp, 460, NULL, NULL, NULL);
            }
        }

        source = it->second.media;
    }
#endif

#if ZC_TEST_SESSION
    if (npt && 0 != source->Seek(*npt)) {
        // 457 Invalid Range
        return rtsp_server_reply_play(rtsp, 457, NULL, NULL, NULL);
    }

    if (scale && 0 != source->SetSpeed(*scale)) {
        // set speed
        assert(*scale > 0);

        // 406 Not Acceptable
        return rtsp_server_reply_play(rtsp, 406, NULL, NULL, NULL);
    }

    // RFC 2326 12.33 RTP-Info (p55)
    // 1. Indicates the RTP timestamp corresponding to the time value in the Range response header.
    // 2. A mapping from RTP timestamps to NTP timestamps (wall clock) is available via RTCP.
    char rtpinfo[512] = {0};
    source->GetRTPInfo(uri, rtpinfo, sizeof(rtpinfo));

    // for vlc 2.2.2
    MP4FileSource *mp4 = dynamic_cast<MP4FileSource *>(source.get());
    if (mp4)
        mp4->SendRTCP(system_clock());

    it->second.status = 1;
#else
    char rtpinfo[512] = {0};
#endif

    return rtsp_server_reply_play(rtsp, 200, npt, NULL, rtpinfo);
}

int CRtspServer::rtsp_onpause(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                              const int64_t *npt) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onpause(ptr, rtsp, uri, session, npt);
}

int CRtspServer::_onpause(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt) {
#if ZC_TEST_SESSION
    std::shared_ptr<IMediaSource> source;
    TSessions::iterator it;
    {
        // AutoThreadLocker locker(s_locker);
        std::lock_guard<std::mutex> locker(m_mutex);
        it = m_sessions.find(session ? session : "");
        if (it == m_sessions.end()) {
            // 454 Session Not Found
            return rtsp_server_reply_pause(rtsp, 454);
        } else {
            // uri with track
            if (0) {
                // 460 Only aggregate operation allowed
                return rtsp_server_reply_pause(rtsp, 460);
            }
        }

        source = it->second.media;
        it->second.status = 2;
    }
#endif

    source->Pause();

    // 457 Invalid Range

    return rtsp_server_reply_pause(rtsp, 200);
}

int CRtspServer::rtsp_onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onteardown(ptr, rtsp, uri, session);
}

int CRtspServer::_onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session) {
#if ZC_TEST_SESSION
    std::shared_ptr<IMediaSource> source;
    TSessions::iterator it;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        it = m_sessions.find(session ? session : "");
        if (it == m_sessions.end()) {
            // 454 Session Not Found
            return rtsp_server_reply_teardown(rtsp, 454);
        }

        source = it->second.media;
        m_sessions.erase(it);
    }
#endif

    return rtsp_server_reply_teardown(rtsp, 200);
}

int CRtspServer::rtsp_onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp, int len) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onannounce(ptr, rtsp, uri, sdp, len);
}

int CRtspServer::_onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp, int len) {
    return rtsp_server_reply_announce(rtsp, 200);
}

int CRtspServer::rtsp_onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                               const double *scale) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onrecord(ptr, rtsp, uri, session, npt, scale);
}

int CRtspServer::_onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                           const double *scale) {
    return rtsp_server_reply_record(rtsp, 200, NULL, NULL);
}

int CRtspServer::rtsp_onoptions(void *ptr, rtsp_server_t *rtsp, const char *uri) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onoptions(ptr, rtsp, uri);
}

int CRtspServer::_onoptions(void *ptr, rtsp_server_t *rtsp, const char *uri) {
    const char *require = rtsp_server_get_header(rtsp, "Require");
    return rtsp_server_reply_options(rtsp, 200);
}

int CRtspServer::rtsp_ongetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                     const void *content, int bytes) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_ongetparameter(ptr, rtsp, uri, session, content, bytes);
}

int CRtspServer::_ongetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                 const void *content, int bytes) {
    const char *ctype = rtsp_server_get_header(rtsp, "Content-Type");
    const char *encoding = rtsp_server_get_header(rtsp, "Content-Encoding");
    const char *language = rtsp_server_get_header(rtsp, "Content-Language");
    return rtsp_server_reply_get_parameter(rtsp, 200, NULL, 0);
}

int CRtspServer::rtsp_onsetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                     const void *content, int bytes) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onsetparameter(ptr, rtsp, uri, session, content, bytes);
}

int CRtspServer::_onsetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                 const void *content, int bytes) {
    const char *ctype = rtsp_server_get_header(rtsp, "Content-Type");
    const char *encoding = rtsp_server_get_header(rtsp, "Content-Encoding");
    const char *language = rtsp_server_get_header(rtsp, "Content-Language");
    return rtsp_server_reply_set_parameter(rtsp, 200);
}

// ptr2 session
int CRtspServer::rtsp_onclose(void *ptr2) {
    LOG_WARN("rtsp close ptr2[%p]", ptr2);
    // TODO(zhoucc)
    // CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr2);
    // return psvr->_onclose(ptr2);
    return 0;
}

int CRtspServer::_onclose(void *ptr2) {
    // TODO(zhoucc): notify rtsp connection lost
    //       start a timer to check rtp/rtcp activity
    //       close rtsp media session on expired
    printf("rtsp close\n");
    return 0;
}

void CRtspServer::rtsp_onerror(void *ptr, rtsp_server_t *rtsp, int code) {
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_onerror(ptr, rtsp, code);
}

void CRtspServer::_onerror(void *ptr, rtsp_server_t *rtsp, int code) {
    printf("rtsp_onerror code=%d, rtsp=%p\n", code, rtsp);
#if ZC_TEST_SESSION
    TSessions::iterator it;
    std::lock_guard<std::mutex> locker(m_mutex);
    for (it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        if (rtsp == it->second.rtsp) {
            it->second.media->Pause();
            m_sessions.erase(it);
            break;
        }
    }
#endif

    return;
}

int CRtspServer::rtsp_send(void *ptr, const void *data, size_t bytes) {
    // TODO(zhoucc)
    CRtspServer *psvr = reinterpret_cast<CRtspServer *>(ptr);
    return psvr->_send(ptr, data, bytes);
}

int CRtspServer::_send(void *ptr, const void *data, size_t bytes) {
    socket_t socket = (socket_t)(intptr_t)ptr;

    // TODO(zhoucc): send multiple rtp packet once time;unuse ??
    return bytes == socket_send(socket, data, bytes, 0) ? 0 : -1;
}

CRtspServer::CRtspServer() : Thread("RtspSer"), m_init(false), m_running(0), m_phandle(nullptr), m_psvr(nullptr) {
    memset(&m_cbinfo, 0, sizeof(m_cbinfo));
}

CRtspServer::~CRtspServer() {
    UnInit();
}

bool CRtspServer::_init() {
    aio_worker_init(ZC_N_AIO_THREAD);
    void *tcp = nullptr;
    struct aio_rtsp_handler_t *phandler = (struct aio_rtsp_handler_t *)malloc(sizeof(struct aio_rtsp_handler_t));
    ZC_ASSERT(phandler);
    if (phandler == NULL) {
        LOG_ERROR("malloc error");
        goto _err;
    }
    memset(phandler, 0, sizeof(*phandler));

    phandler->base.ondescribe = rtsp_ondescribe;
    phandler->base.onsetup = rtsp_onsetup;
    phandler->base.onplay = rtsp_onplay;
    phandler->base.onpause = rtsp_onpause;
    phandler->base.onteardown = rtsp_onteardown;
    phandler->base.close = rtsp_onclose;
    phandler->base.onannounce = rtsp_onannounce;
    phandler->base.onrecord = rtsp_onrecord;
    phandler->base.onoptions = rtsp_onoptions;
    phandler->base.ongetparameter = rtsp_ongetparameter;
    phandler->base.onsetparameter = rtsp_onsetparameter;
    phandler->base.send = nullptr;  // ignore rtsp_send;
    phandler->onerror = rtsp_onerror;

    // 1. check s_workdir, MUST be end with '/' or '\\'
    // 2. url: rtsp://127.0.0.1:8554/vod/<filename>
    // void *tcp = rtsp_server_listen("0.0.0.0", 8554, phandler, NULL);
    tcp = rtsp_server_listen("0.0.0.0", 8554, phandler, this);
    ZC_ASSERT(tcp);
    if (tcp == NULL) {
        LOG_ERROR("rtsp_server_listen error");
        goto _err;
    }
    LOG_WARN("listen init this[%p] tcplisten[%p]", this, tcp);
    // void* udp = rtsp_transport_udp_create(NULL, 554, phandler, NULL);
    // ZC_ASSERT(udp);

    m_phandle = phandler;
    m_psvr = tcp;
    LOG_TRACE("rtspserver _init ok");
    return true;
_err:
    if (tcp)
        rtsp_server_unlisten(tcp);
    ZC_SAFE_FREE(phandler);
    aio_worker_clean(ZC_N_AIO_THREAD);
    LOG_ERROR("rtspserver _init error");
    return false;
}

bool CRtspServer::Init(rtspsvr_cb_info_t *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    if (!_init()) {
        goto _err;
    }
    memcpy(&m_cbinfo, cbinfo, sizeof(m_cbinfo));
    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    _unInit();

    LOG_TRACE("Init error");
    return false;
}

bool CRtspServer::_unInit() {
    Stop();
    rtsp_server_unlisten(m_psvr);
    m_psvr = nullptr;
    ZC_SAFE_FREE(m_phandle);
    aio_worker_clean(ZC_N_AIO_THREAD);
    return true;
}

bool CRtspServer::UnInit() {
    if (!m_init) {
        return false;
    }

    _unInit();

    m_init = false;
    return true;
}

bool CRtspServer::_aio_work() {
    TSessions::iterator it;
    std::lock_guard<std::mutex> locker(m_mutex);
    for (it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        rtsp_media_t &session = it->second;
        if (1 == session.status)
            session.media->Play();
    }

    return true;
}

int CRtspServer::process() {
    LOG_WARN("process into");
    while (State() == Running /*&&  i < loopcnt*/) {
        _aio_work();
        system_sleep(100);
    }
    LOG_WARN("process exit");
    return -1;
}

int CRtspServer::RtspMgrStreamUpdate(unsigned int type, unsigned int chn) {
    LOG_TRACE("CRtspServer, StreamUpdate type:%u, chn:%u", type, chn);
    std::string filename;

    const char *path = zc_get_livestreampath((zc_shmstream_e)type);
    if (!path) {
        LOG_ERROR("error zc_get_livestreampath, type:%u, chn:%u", type);
        return -1;
    }

    filename = path;
    filename += ".ch";
    filename += std::to_string(chn);
    LOG_TRACE("CRtspServer, StreamUpdate %s", filename.c_str());
    {
        std::map<std::string, TFileDescription>::const_iterator it;
        std::lock_guard<std::mutex> locker(m_mutex);
        it = m_describes.find(filename);
        if (it != m_describes.end()) {
            LOG_TRACE("remove, describe type:%u, chn:%u, filename:%s", type, chn, filename.c_str());
            m_describes.erase(it);
        }
    }

    return 0;
}

}  // namespace zc
