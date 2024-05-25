// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <asm-generic/errno.h>
#include <stdio.h>

#include <memory>

#include "rtp-payload.h"
#include "rtp-profile.h"
#include "rtp.h"
#include "sys/path.h"
#include "sys/system.h"

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcLiveSource.hpp"
#include "ZcMediaTrack.hpp"

extern "C" uint32_t rtp_ssrc(void);

#define VIDEO_BANDWIDTH (4 * 1024)
#define AUDIO_BANDWIDTH (4 * 1024)  // bandwidth

namespace zc {
CMediaTrack::CMediaTrack(media_track_e track, int code)
    : m_create(false), m_track(track), m_code(code), m_fiforeader(nullptr), m_rtppacker(nullptr), m_rtp(nullptr),
      m_evfd(0), m_sendcnt(0), m_pollcnt(0), m_rtp_clock(0), m_rtcp_clock(0) {
    memset(m_packet, 0, sizeof(m_packet));
#if ZC_DEBUG_MEDIATRACK
    m_debug_framecnt = 0;
    m_debug_framecnt_last = 0;
    m_debug_cnt_lasttime = 0;
#endif
}

CMediaTrack::~CMediaTrack() {
    UnInit();
}

/*
int CMediaTrack::Create(int codetype, media_info_t *pinfo) {
    int payload = RTP_PAYLOAD_H264;
    int bandwidth = VIDEO_BANDWIDTH;
    int frequence = 90000;
    char name[32];
    char rtpname[32];
    media_info_t info;
    if (m_track == MEDIA_TRACK_AUDIO && pinfo) {
        LOG_WARN("user audio_info chn[%d], bits[%d], rate[%d]", pinfo->audio_info.channels,
                 pinfo->audio_info.sample_bits, pinfo->audio_info.sample_rate);
        memcpy(&info, pinfo, sizeof(media_info_t));
    } else {
        memcpy(&info, &m_meidainfo, sizeof(media_info_t));
    }

    uint32_t ssrc = rtp_ssrc();
    static struct rtp_payload_t s_rtpfunc = {
        CMediaTrack::RTPAlloc,
        CMediaTrack::RTPFree,
        CMediaTrack::RTPPacket,
    };

    struct rtp_event_t event;

    event.on_rtcp = CMediaTrack::OnRTCPEvent;

    // TODO(zhoucc) this
    if (m_track == MEDIA_TRACK_VIDEO) {
        if (codetype == MEDIA_CODE_H264) {
            payload = RTP_PAYLOAD_H264;
            snprintf(name, sizeof(name) - 1, "h264");
            snprintf(rtpname, sizeof(rtpname) - 1, "live.h264");
            // video
            static const char *video_pattern =
                "m=video 0 RTP/AVP %d\n"
                "a=rtpmap:%d H264/90000\n"
                "a=fmtp:%d profile-level-id=%02X%02X%02X;packetization-mode=1;sprop-parameter-sets=";

        } else if (codetype == MEDIA_CODE_H264) {
            payload = RTP_PAYLOAD_H265;
            snprintf(name, sizeof(name) - 1, "h265");
            snprintf(rtpname, sizeof(rtpname) - 1, "live.h265");
        } else {
            LOG_ERROR("Create video track error codetype[%d]", codetype);
            goto _err;
        }
    } else if (m_track == MEDIA_TRACK_AUDIO) {
        bandwidth = AUDIO_BANDWIDTH;
        frequence = m_meidainfo.audio_info.sample_rate;  // 4800
        if (codetype == MEDIA_CODE_AAC) {
            payload = RTP_PAYLOAD_MP4A;
            frequence = m_meidainfo.audio_info.sample_rate;
            bandwidth = AUDIO_BANDWIDTH;
            snprintf(name, sizeof(name) - 1, "mpeg4-generic");
            snprintf(rtpname, sizeof(rtpname) - 1, "live.aac");
        } else {
            LOG_ERROR("Create video track error codetype[%d]", codetype);
            goto _err;
        }
    } else if (m_track == MEDIA_TRACK_META) {
        if (codetype == MEDIA_CODE_METADATA) {
            LOG_WARN("Create metadata track TODO codetype[%d]", codetype);
            // TODO(zhoucc)
        } else {
            LOG_ERROR("Create video track error codetype[%d]", codetype);
            goto _err;
        }
    } else {
        LOG_ERROR("error m_track[%d]", m_track);
        goto _err;
    }

    m_rtppacker = rtp_payload_encode_create(payload, name, (uint16_t)ssrc, ssrc, &s_rtpfunc, this);
    if (m_rtppacker) {
        LOG_ERROR("Create playload encode error codetype[%d]", codetype);
        goto _err;
    }

    m_rtp = rtp_create(&event, this, ssrc, ssrc, frequence, bandwidth, 1);
    if (m_rtp) {
        LOG_ERROR("Create video track error codetype[%d]", codetype);
        goto _err;
    }

    rtp_set_info(m_rtp, "RTSPServer", rtpname);

    LOG_TRACE("Create ok m_track[%d],codetype[%d]", m_track, codetype);
    return 0;
_err:
    LOG_ERROR("Create error m_track[%d],codetype[%d]", m_track, codetype);
        if (m_rtp) {
            rtp_destroy(m_rtp);
            m_rtp = nullptr;
        }

        if (m_rtppacker) {
            rtp_payload_encode_destroy(m_rtppacker);
            m_rtppacker = nullptr;
        }

    ZC_SAFE_DELETE(m_fiforeader);
    return false;
}
*/

void CMediaTrack::UnInit() {
    if (m_create) {
        if (m_rtp) {
            rtp_destroy(m_rtp);
            m_rtp = nullptr;
        }

        if (m_rtppacker) {
            rtp_payload_encode_destroy(m_rtppacker);
            m_rtppacker = nullptr;
        }

        ZC_SAFE_DELETE(m_fiforeader);
        m_create = false;
    }

    return;
}

int CMediaTrack::SetTransport(std::shared_ptr<IRTPTransport> transport) {
    m_transport = transport;
    return 0;
}

int CMediaTrack::GetSDPMedia(std::string &sdp) const {
    sdp = m_sdp;
    return 0;
}

int CMediaTrack::GetRTPInfo(const char *uri, char *rtpinfo, size_t bytes) const {
    uint16_t seq;
    uint32_t timestamp;

    rtp_payload_encode_getinfo(m_rtppacker, &seq, &timestamp);
    snprintf(rtpinfo, bytes, "url=%s/track%d;seq=%hu;rtptime=%u", uri, m_track, seq, timestamp);
    // snprintf(rtpinfo, bytes, "url=%s/track%d;seq=%hu;rtptime=%u", uri, m_track, seq,
    //          (unsigned int)(m_timestamp * (m_frequency / 1000) /*kHz*/));

    return 0;
}

int CMediaTrack::SendBye() {
    char rtcp[1024] = {0};
    size_t n = rtp_rtcp_bye(m_rtp, rtcp, sizeof(rtcp));
    // send RTCP packet
    m_transport->Send(true, rtcp, n);

    return 0;
}

int CMediaTrack::SendRTCP(uint64_t clock) {
    // make sure have sent RTP packet
    int interval = rtp_rtcp_interval(m_rtp);
    if (0 == m_rtcp_clock || m_rtcp_clock + interval < clock) {
        char rtcp[1024] = {0};
        size_t n = rtp_rtcp_report(m_rtp, rtcp, sizeof(rtcp));

        // send RTCP packet
        m_transport->Send(true, rtcp, n);

        m_rtcp_clock = clock;
    }

    return 0;
}

void CMediaTrack::_onRTCPEvent(const struct rtcp_msg_t *msg) {
    // TODO(zhoucc):
    // msg;
    return;
}

void CMediaTrack::OnRTCPEvent(void *param, const struct rtcp_msg_t *msg) {
    CMediaTrack *self = reinterpret_cast<CMediaTrack *>(param);
    return self->_onRTCPEvent(msg);
}

void *CMediaTrack::_RTPAlloc(int bytes) {
    ZC_ASSERT(bytes <= sizeof(m_packet));
    return m_packet;
}

void *CMediaTrack::RTPAlloc(void *param, int bytes) {
    CMediaTrack *self = reinterpret_cast<CMediaTrack *>(param);
    return self->_RTPAlloc(bytes);
}

void CMediaTrack::_RTPFree(void *packet) {
    ZC_ASSERT(m_packet == packet);
}

void CMediaTrack::RTPFree(void *param, void *packet) {
    CMediaTrack *self = reinterpret_cast<CMediaTrack *>(param);
    return self->_RTPFree(packet);
}

int CMediaTrack::_RTPPacket(const void *packet, int bytes, uint32_t timestamp, int msg_flags) {
    ZC_ASSERT(m_packet == packet);

    // Hack: Send an initial RTCP "SR" packet, before the initial RTP packet,
    // so that receivers will (likely) be able to get RTCP-synchronized presentation times immediately:
    rtp_onsend(m_rtp, packet, bytes /*, time*/);
    SendRTCP(system_clock());

    int r = m_transport->Send(false, packet, bytes);
    // ZC_ASSERT(r == (int)bytes);
    if (r != bytes) {
        return -1;
    }

    return 0;
}

int CMediaTrack::RTPPacket(void *param, const void *packet, int bytes, uint32_t timestamp, int msg_flags) {
    CMediaTrack *self = reinterpret_cast<CMediaTrack *>(param);

    return self->_RTPPacket(packet, bytes, timestamp, msg_flags);
}

int CMediaTrack::GetData2Send() {
    int ret = 0;
    static int retcnt = 0;
    zc_frame_t *pframe = (zc_frame_t *)m_framebuf;
    // int buf[1024];
    // if (read(m_fiforeader->GetEvFd(), buf, sizeof(buf)) <= 0) {
    //     LOG_ERROR("epoll wait ok but read error ret[%d], fd[%d]", ret, m_fiforeader->GetEvFd());
    //     m_fiforeader->CloseEvFd();
    //     return -1;
    // }

    if (!m_fiforeader->IsEmpty()) {
#if 0
        ret = m_fiforeader->Get(m_framebuf, sizeof(m_framebuf));
        // LOG_TRACE("get fifodata ret[%d]", ret);
        retcnt += ret;
        rtp_payload_encode_input(m_rtppacker, m_framebuf, (int)ret, m_timestamp * 90 /*kHz*/);
#else
        ret = m_fiforeader->Get(m_framebuf, sizeof(m_framebuf));
        ZC_ASSERT(ret > sizeof(zc_frame_t));
        ZC_ASSERT(pframe->size > 0);
        if (pframe->keyflag) {
            struct timespec _ts;
            clock_gettime(CLOCK_MONOTONIC, &_ts);
            unsigned int now = _ts.tv_sec*1000 + _ts.tv_nsec/1000000;
            LOG_TRACE("get fifodata len[%d],key[%d], pts[%u] utc[%u], cos[%d]",  pframe->keyflag, pframe->size, pframe->size, pframe->utc, now-pframe->utc);
        }
        retcnt += ret;
        rtp_payload_encode_input(m_rtppacker, pframe->data, (int)pframe->size, m_timestamp * 90 /*kHz*/);
#endif
        // m_rtp_clock += 1000/14;
        // m_timestamp += 1000/14;
        m_rtp_clock += 40;
        m_timestamp += 40;
#if ZC_DEBUG_MEDIATRACK
        uint64_t now = system_clock();
        m_debug_framecnt++;
        if (now > (m_debug_cnt_lasttime + 1000 * 10)) {
            LOG_WARN("fps[%.2f],cnt[%u]cos[%llu]",
                     (double)(m_debug_framecnt - m_debug_framecnt_last) * 1000 / (now - m_debug_cnt_lasttime),
                     m_debug_framecnt - m_debug_framecnt_last, (now - m_debug_cnt_lasttime) / 1000);
            m_debug_cnt_lasttime = now;
            m_debug_framecnt_last = m_debug_framecnt = 0;
        }

        SendRTCP(now);
#else
        SendRTCP(system_clock());
#endif
    }

    return 0;
}
}  // namespace zc
