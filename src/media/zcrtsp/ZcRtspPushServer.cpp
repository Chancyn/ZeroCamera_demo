// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// start media-server
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "media/ZcLiveTestWriterSys.hpp"
#include "media/ZcMediaReceiver.hpp"
#include "media/ZcMediaTrack.hpp"
#include "zc_log.h"
#include "zc_macros.h"

#include "aio-socket.h"
#include "aio-worker.h"

#include "cstringext.h"  // strstartswith
#include "ntp-time.h"
#include "path.h"
#include "rtsp-header-transport.h"
#include "sockpair.h"

#include "rtsp-server-aio.h"
#include "sys/system.h"  // system_clock
#include "uri-parse.h"
#include "urlcodec.h"
#include "zc_type.h"

#include "ZcRtspPushServer.hpp"
#include "ZcType.hpp"
#include "media/ZcLiveSource.hpp"
#include "media/ZcMediaReceiverFac.hpp"

#define ZC_N_AIO_THREAD (4)  // aio thread num

namespace zc {
int CRtspPushServer::onframe(void *ptr1, void *ptr2, int encode, const void *packet, int bytes, uint32_t time,
                             int flags) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr1);
    return psvr->_onframe(ptr2, encode, packet, bytes, time, flags);
}

int CRtspPushServer::_onframe(void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags) {
    // LOG_TRACE("encode:%d, time:%08u, flags:%08d, drop.", encode, time, flags);
    CMediaReceiver *precv = reinterpret_cast<CMediaReceiver *>(ptr2);
    return precv->RtpOnFrameIn(packet, bytes, time, flags);
}

int CRtspPushServer::rtsp_uri_parse(const char *uri, std::string &path) {
    char path1[256];
    struct uri_t *r = uri_parse(uri, strlen(uri));
    if (!r)
        return -1;

    url_decode(r->path, strlen(r->path), path1, sizeof(path1));
    path = path1;
    uri_free(r);
    return 0;
}

int CRtspPushServer::rtsp_onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp, int len) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onannounce(rtsp, uri, sdp, len);
}

int CRtspPushServer::_onannounce(rtsp_server_t *rtsp, const char *uri, const char *sdp, int len) {
    std::string filename;
    TPushSessions::const_iterator it;
    std::shared_ptr<pushrtsp_source_t> source(new pushrtsp_source_t);

    rtsp_uri_parse(uri, filename);

    source->count = rtsp_media_sdp(sdp, len, source->media, sizeof(source->media) / sizeof(source->media[0]));
    if (source->count < 0 || source->count > sizeof(source->media) / sizeof(source->media[0]))
        return rtsp_server_reply_announce(rtsp, 451);  // Parameter Not Understood

    const char *contentBase = rtsp_server_get_header(rtsp, "Content-Base");
    const char *contentLocation = rtsp_server_get_header(rtsp, "Content-Location");
    for (int i = 0; i < source->count; i++) {
        if (source->media[i].avformat_count < 1) {
            assert(0);
            // 451 Parameter Not Understood
            return rtsp_server_reply_announce(rtsp, 451);
        }

        // rfc 2326 C.1.1 Control URL (p80)
        // If found at the session level, the attribute indicates the URL for aggregate control
        rtsp_media_set_url(source->media + i, contentBase, contentLocation, uri);
    }

    {
        // AutoThreadLocker locker(s_locker);
        std::lock_guard<std::mutex> locker(m_pushmutex);
        // TODO:
        // 1. checkout source count
        // 2. delete unused source
        m_pushsources[uri] = source;
    }

    return rtsp_server_reply_announce(rtsp, 200);
}

int CRtspPushServer::_pushrtsp_find_media(const char *uri, std::shared_ptr<pushrtsp_source_t> &source) {
    std::map<std::string, std::shared_ptr<pushrtsp_source_t>>::const_iterator it;
    for (it = m_pushsources.begin(); it != m_pushsources.end(); ++it) {
        source = it->second;
        for (int i = 0; i < source->count; i++) {
            if (0 == strcmp(source->media[i].uri, uri))
                return i;
        }
    }

    return -1;
}

int CRtspPushServer::rtsp_onsetup(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                  const struct rtsp_header_transport_t transports[], size_t num) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onsetup(rtsp, uri, session, transports, num);
}

int CRtspPushServer::_onsetup(rtsp_server_t *rtsp, const char *uri, const char *session,
                              const struct rtsp_header_transport_t transports[], size_t num) {
    std::string filename;
    char rtsp_transport[128] = {0};

    rtsp_uri_parse(uri, filename);

    std::shared_ptr<pushrtsp_source_t> source;
    int i = _pushrtsp_find_media(uri, source);
    if (-1 == i)
        return rtsp_server_reply_setup(rtsp, 404 /*Not Found*/, NULL, NULL);

    std::shared_ptr<pushrtsp_stream_t> stream(new pushrtsp_stream_t);
    stream->media.reset(new rtsp_media_t);
    memcpy(stream->media.get(), source->media + i, sizeof(struct rtsp_media_t));

    TPushSessions::iterator it;
    // AutoThreadLocker locker(s_locker);
    std::lock_guard<std::mutex> locker(m_pushmutex);
    if (session) {
        it = m_pushsessions.find(session);
        if (it == m_pushsessions.end()) {
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
        char rtspsession[32];
        std::shared_ptr<pushrtsp_session_t> item(new pushrtsp_session_t);
        snprintf(rtspsession, sizeof(rtspsession), "%p", item.get());
        it = m_pushsessions.insert(std::make_pair(rtspsession, item)).first;
    }

    for (size_t i = 0; i < num; i++) {
        const struct rtsp_header_transport_t *t = transports + i;
        if (RTSP_TRANSPORT_RTP_TCP == transports[i].transport) {
            // RTP/AVP/TCP
            // 10.12 Embedded (Interleaved) Binary Data (p40)
            stream->tack = i;  // save trackid
            stream->trackcode = CRtpReceiver::Encodingtrans2Type(stream->media->avformats[0].encoding,
                                                                 stream->tracktype, stream->encode);
            memcpy(&stream->transport, t, sizeof(rtsp_header_transport_t));
            // RTP/AVP/TCP;interleaved=0-1
            snprintf(rtsp_transport, sizeof(rtsp_transport), "RTP/AVP/TCP;interleaved=%d-%d",
                     stream->transport.interleaved1, stream->transport.interleaved2);
            break;
        } else if (RTSP_TRANSPORT_RTP_UDP == transports[i].transport) {
            stream->tack = i;  // save trackid
            stream->trackcode = CRtpReceiver::Encodingtrans2Type(stream->media->avformats[0].encoding,
                                                                 stream->tracktype, stream->encode);
            // RTP/AVP/UDP
            memcpy(&stream->transport, t, sizeof(rtsp_header_transport_t));

            if (t->multicast) {
                // RFC 2326 1.6 Overall Operation p12
                // Multicast, client chooses address
                // Multicast, server chooses address
                assert(0);
                // 461 Unsupported Transport
                return rtsp_server_reply_setup(rtsp, 461, NULL, NULL);
            } else {
                // unicast
                unsigned short port[2];
                // 0.0.0.0 set recvfrom any
                if (0 != sockpair_create("0.0.0.0", stream->socket, port)) {
                    // 500 Internal Server Error
                    return rtsp_server_reply_setup(rtsp, 500, NULL, NULL);
                }
                stream->tack = i;  // save trackid
                assert(stream->transport.rtp.u.client_port1 && stream->transport.rtp.u.client_port2);
                if (0 == stream->transport.destination[0])
                    snprintf(stream->transport.destination, sizeof(stream->transport.destination), "%s",
                             rtsp_server_get_client(rtsp, NULL));

                // RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257;destination=xxxx
                snprintf(rtsp_transport, sizeof(rtsp_transport),
                         "RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu%s%s", t->rtp.u.client_port1,
                         t->rtp.u.client_port2, port[0], port[1], t->destination[0] ? ";destination=" : "",
                         t->destination[0] ? t->destination : "");
                // LOG_TRACE("socket%d-%d,port%hu-%hu", stream->socket[0], stream->socket[1], port[0], port[1]);
            }
            break;
        } else {
            // 461 Unsupported Transport
            // try next
        }
    }
    if (0 == rtsp_transport[0]) {
        // 461 Unsupported Transport
        return rtsp_server_reply_setup(rtsp, 461, NULL, NULL);
    }

    it->second->status = 0;
    it->second->streams.push_back(stream);
    return rtsp_server_reply_setup(rtsp, 200, it->first.c_str(), rtsp_transport);
}

int CRtspPushServer::rtsp_onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                   const int64_t *npt, const double *scale) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onrecord(rtsp, uri, session, npt, scale);
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

int CRtspPushServer::_onrecord(rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t * /*npt*/,
                               const double * /*scale*/) {
    int nchn = 0;
    const char *pchn = nullptr;
    std::string filename;
    rtsp_uri_parse(uri, filename);
    if (strstartswith(filename.c_str(), "/push/")) {
        filename = filename.c_str() + 6;
    }
    pchn = strstr(filename.c_str(), "ch");
    if (pchn != nullptr) {
        nchn = atoi(pchn + 2);
        nchn = nchn < 0 ? 0 : nchn;
        LOG_TRACE("push server prase nchn:%d, path: %s", filename.c_str());
    }

    LOG_TRACE("pushsvr nchn:%d, path: %s", filename.c_str());
    zc_stream_info_t stinfo = {0};
    int trackidx = 0;

    // get streaminfo
    if (!m_cbinfo.GetInfoCb || m_cbinfo.GetInfoCb(m_cbinfo.MgrContext, nchn, &stinfo) < 0) {
        LOG_ERROR("rtsp_client_play m_cbinfo.GetInfoCb error");
        // 500 Internal Server Error
        return rtsp_server_reply_play(rtsp, 500, NULL, NULL, NULL);
    }

    for (unsigned int i = 0; i < stinfo.tracknum; i++) {
        stinfo.tracks[i].enable = 0;
        LOG_TRACE("diable i:%u, trackno:%u, tracktype:%u", i, stinfo.tracks[i].trackno, stinfo.tracks[i].tracktype);
    }

    std::list<std::shared_ptr<pushrtsp_stream_t>> streams;
    {
        TPushSessions::iterator it;
        // AutoThreadLocker locker(s_locker);
        std::lock_guard<std::mutex> locker(m_pushmutex);
        it = m_pushsessions.find(session ? session : "");
        if (it == m_pushsessions.end()) {
            // 454 Session Not Found
            return rtsp_server_reply_play(rtsp, 454, NULL, NULL, NULL);
        } else {
            // uri with track
            if (0) {
                // 460 Only aggregate operation allowed
                return rtsp_server_reply_play(rtsp, 460, NULL, NULL, NULL);
            }
        }

        it->second->status = 1;
        streams = it->second->streams;
    }

    std::list<std::shared_ptr<pushrtsp_stream_t>>::iterator it;
    for (it = streams.begin(); it != streams.end(); ++it) {
        std::shared_ptr<pushrtsp_stream_t> &stream = *it;
        // find trackidx
        if (stream->trackcode >= 0 && ((trackidx = findTrackIndex(stream->tracktype, stinfo)) >= 0)) {
            // update code

            stinfo.tracks[trackidx].encode = stream->encode;
            stinfo.tracks[trackidx].mediacode =
                CRtpReceiver::transEncode2MediaCode(stream->encode);  // stream->trackcode;
            stinfo.tracks[trackidx].enable = 1;                       // enable track
            LOG_TRACE("prase i:%u, trackno:%u, tracktype:%u, mediacode:%u, encode:%u", trackidx,
                      stinfo.tracks[trackidx].trackno, stinfo.tracks[trackidx].tracktype,
                      stinfo.tracks[trackidx].mediacode, stinfo.tracks[trackidx].encode);
        }
        // media receiver, receiver frame
        if (ZC_MEDIA_CODE_H264 == stream->trackcode) {
            stream->mediarecv.reset(new CMediaReceiverH264(stinfo.tracks[trackidx]));
            stream->mediarecv->Init();
        } else if (ZC_MEDIA_CODE_H265 == stream->trackcode) {
            stream->mediarecv.reset(new CMediaReceiverH265(stinfo.tracks[trackidx]));
            stream->mediarecv->Init();
        } else if (ZC_MEDIA_CODE_AAC == stream->trackcode) {
            stream->mediarecv.reset(new CMediaReceiverAAC(stinfo.tracks[trackidx]));
            stream->mediarecv->Init();
        } else {
            stream->mediarecv.reset();
            // continue;
        }

        stream->rtpreceiver.reset(new CRtpReceiver(onframe, this, stream->mediarecv.get()));
        if (RTSP_TRANSPORT_RTP_UDP == stream->transport.transport) {
            assert(!stream->transport.multicast);
            int port[2] = {stream->transport.rtp.u.client_port1, stream->transport.rtp.u.client_port2};
            // LOG_ERROR("cport:%hu-%hu, sport:%hu-%hu", stream->transport.rtp.u.client_port1,
            //           stream->transport.rtp.u.client_port2, stream->transport.rtp.u.server_port1,
            //           stream->transport.rtp.u.server_port2);
            // TODO(zhoucc): rtp recvicer, receiver rtp package
            // rtp_receiver_test(stream->socket, stream->transport.destination, port, stream->media->avformats[0].fmt,
            //                   stream->media->avformats[0].encoding);
            stream->rtpreceiver->RtpReceiverUdpStart(stream->socket, stream->transport.destination, port,
                                                     stream->media->avformats[0].fmt,
                                                     stream->media->avformats[0].encoding);

        } else if (RTSP_TRANSPORT_RTP_TCP == stream->transport.transport) {
            // assert(0);
            // to be continue
            LOG_ERROR("tcp new into");
            stream->rtpreceiver->RtpReceiverTcpStart(stream->transport.interleaved1, stream->transport.interleaved2,
                                                     stream->media->avformats[0].fmt,
                                                     stream->media->avformats[0].encoding);
        } else {
            assert(0);
        }
    }

    // get streaminfo
    if (!m_cbinfo.SetInfoCb || m_cbinfo.SetInfoCb(m_cbinfo.MgrContext, nchn, &stinfo) < 0) {
        LOG_ERROR("rtsp_client_play m_cbinfo.SetInfoCb error");
        // 500 Internal Server Error
        return rtsp_server_reply_play(rtsp, 500, NULL, NULL, NULL);
    }

    for (unsigned int i = 0; i < stinfo.tracknum; i++) {
        LOG_TRACE("debugset i:%u, trackno:%u, tracktype:%u, mediacode:%u, encode:%u", i, stinfo.tracks[i].trackno,
                  stinfo.tracks[i].tracktype, stinfo.tracks[i].mediacode, stinfo.tracks[i].encode);
    }
    return rtsp_server_reply_record(rtsp, 200, NULL, NULL);
}

int CRtspPushServer::rtsp_onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onteardown(rtsp, uri, session);
}

int CRtspPushServer::_onteardown(rtsp_server_t *rtsp, const char * /*uri*/, const char *session) {
    std::list<std::shared_ptr<pushrtsp_stream_t>> streams;
    {
        TPushSessions::iterator it;
        // AutoThreadLocker locker(s_locker);
        std::lock_guard<std::mutex> locker(m_pushmutex);
        it = m_pushsessions.find(session ? session : "");
        if (it == m_pushsessions.end()) {
            // 454 Session Not Found
            return rtsp_server_reply_play(rtsp, 454, NULL, NULL, NULL);
        } else {
            // uri with track
            if (0) {
                // 460 Only aggregate operation allowed
                return rtsp_server_reply_play(rtsp, 460, NULL, NULL, NULL);
            }
        }

        streams = it->second->streams;
        m_pushsessions.erase(it);
    }

    std::list<std::shared_ptr<pushrtsp_stream_t>>::iterator it;
    for (it = streams.begin(); it != streams.end(); ++it) {
        std::shared_ptr<pushrtsp_stream_t> &stream = *it;
    }

    return rtsp_server_reply_teardown(rtsp, 200);
}
#if 0
int CRtspPushServer::rtsp_onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onteardown(rtsp, uri, session);
}

int CRtspPushServer::_onteardown(rtsp_server_t *rtsp, const char *uri, const char *session) {
    std::shared_ptr<IMediaSource> source;
    TPushSessions::iterator it;
    {
        std::lock_guard<std::mutex> locker(m_pushmutex);
        it = m_pushsessions.find(session ? session : "");
        if (it == m_pushsessions.end()) {
            // 454 Session Not Found
            return rtsp_server_reply_teardown(rtsp, 454);
        }

        source = it->second.media;
        m_pushsessions.erase(it);
    }

    return rtsp_server_reply_teardown(rtsp, 200);
}


int CRtspPushServer::rtsp_onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp, int len) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onannounce(rtsp, uri, sdp, len);
}

int CRtspPushServer::_onannounce(rtsp_server_t *rtsp, const char *uri, const char *sdp, int len) {
    return rtsp_server_reply_announce(rtsp, 200);
}

int CRtspPushServer::rtsp_onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                   const int64_t *npt, const double *scale) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onrecord(rtsp, uri, session, npt, scale);
}

int CRtspPushServer::_onrecord(rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                               const double *scale) {
    return rtsp_server_reply_record(rtsp, 200, NULL, NULL);
}
#endif

int CRtspPushServer::rtsp_onoptions(void *ptr, rtsp_server_t *rtsp, const char *uri) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onoptions(rtsp, uri);
}

int CRtspPushServer::_onoptions(rtsp_server_t *rtsp, const char *uri) {
    const char *require = rtsp_server_get_header(rtsp, "Require");
    return rtsp_server_reply_options(rtsp, 200);
}

int CRtspPushServer::rtsp_ongetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                         const void *content, int bytes) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_ongetparameter(rtsp, uri, session, content, bytes);
}

int CRtspPushServer::_ongetparameter(rtsp_server_t *rtsp, const char *uri, const char *session, const void *content,
                                     int bytes) {
    const char *ctype = rtsp_server_get_header(rtsp, "Content-Type");
    const char *encoding = rtsp_server_get_header(rtsp, "Content-Encoding");
    const char *language = rtsp_server_get_header(rtsp, "Content-Language");
    return rtsp_server_reply_get_parameter(rtsp, 200, NULL, 0);
}

int CRtspPushServer::rtsp_onsetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                         const void *content, int bytes) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onsetparameter(rtsp, uri, session, content, bytes);
}

int CRtspPushServer::_onsetparameter(rtsp_server_t *rtsp, const char *uri, const char *session, const void *content,
                                     int bytes) {
    const char *ctype = rtsp_server_get_header(rtsp, "Content-Type");
    const char *encoding = rtsp_server_get_header(rtsp, "Content-Encoding");
    const char *language = rtsp_server_get_header(rtsp, "Content-Language");
    return rtsp_server_reply_set_parameter(rtsp, 200);
}

// ptr2 session
int CRtspPushServer::rtsp_onclose(void *ptr2) {
    LOG_WARN("rtsp close ptr2[%p]", ptr2);
    // TODO(zhoucc)
    // CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr2);
    // return psvr->_onclose(ptr2);
    return 0;
}

int CRtspPushServer::_onclose(void *ptr2) {
    // TODO(zhoucc): notify rtsp connection lost
    //       start a timer to check rtp/rtcp activity
    //       close rtsp media session on expired
    printf("rtsp close\n");
    return 0;
}

void CRtspPushServer::rtsp_onerror(void *ptr, rtsp_server_t *rtsp, int code) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onerror(rtsp, code);
}

void CRtspPushServer::_onerror(rtsp_server_t *rtsp, int code) {
    LOG_ERROR("rtsp_onerror code=%d, rtsp=%p, rtsp->session=%p,  \n", code, rtsp, rtsp->session);
    TPushSessions::iterator it;

    // TODO(zhoucc): Teardown

    std::lock_guard<std::mutex> locker(m_pushmutex);
    // for (it = m_pushsessions.begin(); it != m_pushsessions.end(); ++it) {
    //     if (rtsp == it->second.rtsp) {
    //         it->second.media->Pause();
    //         m_pushsessions.erase(it);
    //         break;
    //     }
    // }

    return;
}

void CRtspPushServer::rtsp_onerror2(void *ptr, rtsp_server_t *rtsp, int code, void *ptr2) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_onerror2(rtsp, code, ptr2);
}

void CRtspPushServer::_onerror2(rtsp_server_t *rtsp, int code, void *ptr2) {
    // ptr2[session]
    LOG_ERROR("rtsp_onerror2 code=%d, rtsp=%p, rtsp.session=%p, ptr2[%p]\n", code, rtsp, rtsp->session, ptr2);
    TPushSessions::iterator it;
    std::lock_guard<std::mutex> locker(m_pushmutex);
    // for (it = m_pushsessions.begin(); it != m_pushsessions.end(); ++it) {
    //     if (rtsp == it->second.rtsp) {
    //         it->second.media->Pause();
    //         m_pushsessions.erase(it);
    //         break;
    //     }
    // }

    return;
}

int CRtspPushServer::rtsp_send(void *ptr, const void *data, size_t bytes) {
    // TODO(zhoucc)
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(ptr);
    return psvr->_send(data, bytes);
}

int CRtspPushServer::_send(const void *data, size_t bytes) {
    // socket_t socket = (socket_t)(intptr_t)ptr;

    // // TODO(zhoucc): send multiple rtp packet once time;unuse
    // return bytes == socket_send(socket, data, bytes, 0) ? 0 : -1;
    return 0;
}

void CRtspPushServer::onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(param);
    return psvr->_onrtp(channel, data, bytes);
}

void CRtspPushServer::_onrtp(uint8_t channel, const void *data, uint16_t bytes) {
    LOG_ERROR("_onrtp channel:%u, bytes:%hu", channel, bytes);
    // ZC_ASSERT(channel <= ZC_MEIDIA_NUM);
    // stream->rtpreceiver->RtpReceiverTcpInput(channel, data, bytes);
}

void CRtspPushServer::onrtp2(void *param, uint8_t channel, const void *data, uint16_t bytes, void *ptr2) {
    CRtspPushServer *psvr = reinterpret_cast<CRtspPushServer *>(param);
    return psvr->_onrtp2(channel, data, bytes, ptr2);
}

void CRtspPushServer::_onrtp2(uint8_t channel, const void *data, uint16_t bytes, void *session) {
    // ZC_ASSERT(ptr2 != nullptr);
    // struct rtsp_session_t *session = (struct rtsp_session_t *)ptr2;
    const char *sessionstr = (const char *)session;

    // ZC_ASSERT(sessionstr[0] != '\0');
    if (sessionstr[0] == '\0') {
        return;
    }
    // LOG_TRACE("_onrtp channel:%u, bytes:%hu, this[%p], sessionstr[%s]", channel, bytes, this, sessionstr);
    if (channel % 2) {
        // TODO(zhoucc): rtcp channel ,maybe recv rtcp data? maybe todo
        return;
    }
    std::list<std::shared_ptr<pushrtsp_stream_t>> streams;
    {
        TPushSessions::iterator it;
        // AutoThreadLocker locker(s_locker);
        std::lock_guard<std::mutex> locker(m_pushmutex);
        it = m_pushsessions.find(sessionstr);
        if (it != m_pushsessions.end()) {
            streams = it->second->streams;
        }
    }

    std::list<std::shared_ptr<pushrtsp_stream_t>>::iterator it;
    for (it = streams.begin(); it != streams.end(); ++it) {
        std::shared_ptr<pushrtsp_stream_t> &stream = *it;
        if (stream->tack == channel / 2) {
            stream->rtpreceiver->RtpReceiverTcpInput(channel, data, bytes);
            break;
        }
    }
}

CRtspPushServer::CRtspPushServer()
    : Thread("RtspSer"), m_init(false), m_running(0), m_phandle(nullptr), m_psvr(nullptr) {}

CRtspPushServer::~CRtspPushServer() {
    UnInit();
}

bool CRtspPushServer::_init() {
    aio_worker_init(ZC_N_AIO_THREAD);
    void *tcp = nullptr;
    struct aio_rtsp_handler_t *phandler = (struct aio_rtsp_handler_t *)malloc(sizeof(struct aio_rtsp_handler_t));
    ZC_ASSERT(phandler);
    if (phandler == NULL) {
        LOG_ERROR("malloc error");
        goto _err;
    }
    memset(phandler, 0, sizeof(*phandler));

    // phandler->base.ondescribe = rtsp_ondescribe;
    phandler->base.onsetup = rtsp_onsetup;
    // phandler->base.onplay = rtsp_onplay;
    // phandler->base.onpause = rtsp_onpause;
    phandler->base.onteardown = rtsp_onteardown;
    phandler->base.close = rtsp_onclose;
    phandler->base.onannounce = rtsp_onannounce;
    phandler->base.onrecord = rtsp_onrecord;
    phandler->base.onoptions = rtsp_onoptions;
    phandler->base.ongetparameter = rtsp_ongetparameter;
    phandler->base.onsetparameter = rtsp_onsetparameter;
    phandler->base.send = nullptr;  // ignore rtsp_send;
    phandler->onerror = rtsp_onerror;

    // pushserver
    phandler->onrtp = onrtp;
    phandler->onrtp2 = onrtp2;
    phandler->onerror2 = rtsp_onerror2;
    // 1. check s_workdir, MUST be end with '/' or '\\'
    // 2. url: rtsp://127.0.0.1:8554/vod/<filename>
    // void *tcp = rtsp_server_listen("0.0.0.0", 8554, phandler, NULL);
    tcp = rtsp_server_listen("0.0.0.0", 5540, phandler, this);
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

bool CRtspPushServer::Init(rtsppushs_callback_info_t *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }
    memcpy(&m_cbinfo, cbinfo, sizeof(rtsppushs_callback_info_t));
    if (!_init()) {
        goto _err;
    }
    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    _unInit();

    LOG_TRACE("Init error");
    return false;
}

bool CRtspPushServer::_unInit() {
    Stop();
    rtsp_server_unlisten(m_psvr);
    m_psvr = nullptr;
    ZC_SAFE_FREE(m_phandle);
    aio_worker_clean(ZC_N_AIO_THREAD);
    return false;
}

bool CRtspPushServer::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}

bool CRtspPushServer::_aio_work() {
    // TODO(zhoucc):

    return true;
}

int CRtspPushServer::process() {
    LOG_WARN("process into\n");
    while (State() == Running /*&&  i < loopcnt*/) {
        _aio_work();
        system_sleep(100);
    }
    LOG_WARN("process exit\n");
    return -1;
}
}  // namespace zc
