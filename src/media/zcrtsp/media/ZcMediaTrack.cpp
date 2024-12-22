// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <asm-generic/errno.h>
#include <stdio.h>

#include <memory>

#include "rtp-payload.h"
#include "rtp-profile.h"
#include "rtp.h"
#include "sys/path.h"
//#include "sys/system.h"
#include "zc_basic_fun.h"

#include "ZcType.hpp"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcLiveSource.hpp"
#include "ZcMediaTrack.hpp"

extern "C" uint32_t rtp_ssrc(void);

#define VIDEO_BANDWIDTH (4 * 1024)

namespace zc {
CMediaTrack::CMediaTrack(const zc_meida_track_t &info)
    : m_create(false), m_fiforeader(nullptr), m_rtppacker(nullptr), m_rtp(nullptr), m_evfd(0), m_sendcnt(0),
      m_pollcnt(0), m_rtp_clock(0), m_rtcp_clock(0) {
    memcpy(&m_info, &info, sizeof(zc_meida_track_t));
    memset(m_packet, 0, sizeof(m_packet));
    m_timestamp = 0;
    m_dts_first = -1;
    m_dts_last = -1;
#if ZC_DEBUG_MEDIATRACK
    m_debug_framecnt = 0;
    m_debug_framecnt_last = 0;
    m_debug_cnt_lasttime = 0;
#endif
}

CMediaTrack::~CMediaTrack() {
    UnInit();
}

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
    uint16_t seq = 0;
    uint32_t timestamp = 0;

    rtp_payload_encode_getinfo(m_rtppacker, &seq, &timestamp);
    // snprintf(rtpinfo, bytes, "url=%s/track%d;seq=%hu;rtptime=%u", uri, m_info.trackno, seq, timestamp);
    snprintf(rtpinfo, bytes, "url=%s/track%d;seq=%hu;rtptime=%u", uri, m_info.trackno, seq,
             (unsigned int)(m_timestamp * (m_frequency / 1000) /*kHz*/));
    LOG_WARN("rtptime, tracktype:%d, seq:%hu, timestamp:%u, %s", m_info.tracktype, seq, m_timestamp, rtpinfo);
    return 0;
}

int CMediaTrack::SendBye() {
    char rtcp[1024] = {0};
    size_t n = rtp_rtcp_bye(m_rtp, rtcp, sizeof(rtcp));
    // send RTCP packet
    if (m_transport)
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
        if (m_transport)
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
    ZC_ASSERT(bytes <= (int)sizeof(m_packet));
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
    SendRTCP(zc_system_clock());
    if (m_transport) {
        int r = m_transport->Send(false, packet, bytes);
        // ZC_ASSERT(r == (int)bytes);
        if (r != bytes) {
            return -1;
        }
    }

    return 0;
}

int CMediaTrack::RTPPacket(void *param, const void *packet, int bytes, uint32_t timestamp, int msg_flags) {
    CMediaTrack *self = reinterpret_cast<CMediaTrack *>(param);

    return self->_RTPPacket(packet, bytes, timestamp, msg_flags);
}

int CMediaTrack::GetData2Send() {
    unsigned int ret = 0;
    zc_frame_t *pframe = (zc_frame_t *)m_framebuf;

#if 0  // for test frame write over read
    // system_sleep(100);
    // if (!m_fiforeader->IsEmpty()) {
#endif
    while (m_fiforeader->Len() > sizeof(zc_frame_t)) {
        ret = m_fiforeader->Get(m_framebuf, sizeof(m_framebuf), sizeof(zc_frame_t));
        if (ret < sizeof(zc_frame_t)) {
            return -1;
        }

        ZC_ASSERT(pframe->size > 0);
        if (pframe->video.encode != m_info.encode) {
            if (pframe->keyflag)
                LOG_ERROR("encode error frame:%u!=encode:%u", pframe->video.encode, m_info.encode);
            continue;
        }
        if (pframe->keyflag) {
            uint64_t now = zc_system_time();
            LOG_TRACE("this:%p, rtsp:pts:%u,utc:%u,now:%u,len:%d,cos:%dms", this, pframe->pts, pframe->utc, now, pframe->size,
                      now - pframe->utc);
        }
        bool first = false;
        if (-1 == m_dts_first) {
            m_dts_first = pframe->pts;
            first  = true;
        }

        m_dts_last = pframe->pts;
        uint32_t timestamp = (m_timestamp + (uint32_t)((m_dts_last - m_dts_first))) * (m_frequency / 1000);
        if (first) {
            LOG_WARN("track:%d, size:%d, pts:%d, timestamp:%u, freq:%u, ", m_info.tracktype,  pframe->size, pframe->pts, timestamp, m_frequency);
        }
        //  LOG_WARN("track:%d, size:%d, pts:%u, timestamp:%u, freq:%u, ", m_info.tracktype, pframe->size, pframe->pts, timestamp, m_frequency);
        rtp_payload_encode_input(m_rtppacker, pframe->data, (int)pframe->size, timestamp /*kHz*/);

        // m_rtp_clock += 16;
        // m_timestamp += 16;
#if ZC_DEBUG_MEDIATRACK
        uint64_t now = zc_system_clock();
        m_debug_framecnt++;
        if (now > (m_debug_cnt_lasttime + 1000 * 10)) {
            LOG_TRACE("track:%d, fps[%.2f],cnt[%u]cos[%llu]", m_info.tracktype,
                     (double)(m_debug_framecnt - m_debug_framecnt_last) * 1000 / (now - m_debug_cnt_lasttime),
                     m_debug_framecnt - m_debug_framecnt_last, (now - m_debug_cnt_lasttime) / 1000);
            m_debug_cnt_lasttime = now;
            m_debug_framecnt_last = m_debug_framecnt = 0;
        }

        SendRTCP(now);
#else
        SendRTCP(zc_system_clock());
#endif
    }

    return 0;
}
}  // namespace zc
