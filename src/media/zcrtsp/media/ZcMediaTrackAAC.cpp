// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <asm-generic/errno.h>
#include <stdio.h>

#include "base64.h"
#include "rtp-payload.h"
#include "rtp-profile.h"
#include "rtp.h"
#include "sys/path.h"
#include "sys/system.h"

#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcMediaTrackAAC.hpp"
#include "ZcType.hpp"

#define ZC_AUDIO_BANDWIDTH (4 * 1024)  // bandwidth

extern "C" uint32_t rtp_ssrc(void);

namespace zc {
CMediaTrackAAC::CMediaTrackAAC(const zc_meida_track_t &info)
    : CMediaTrack(info) {
    if (m_info.atinfo.channels) {
        m_info.atinfo.channels = m_info.atinfo.channels ? m_info.atinfo.channels : ZC_AUDIO_CHN;
        m_info.atinfo.sample_bits = m_info.atinfo.sample_bits ? m_info.atinfo.sample_bits : ZC_AUDIO_SAMPLE_BIT_16;
        m_info.atinfo.sample_rate = m_info.atinfo.sample_rate ? m_info.atinfo.sample_rate : ZC_AUDIO_FREQUENCE;
    } else {
        m_info.atinfo.channels = ZC_AUDIO_CHN;
        m_info.atinfo.sample_bits = ZC_AUDIO_SAMPLE_BIT_16;
        m_info.atinfo.sample_rate = ZC_AUDIO_FREQUENCE;
    }

    m_frequency = m_info.atinfo.sample_rate;
    LOG_TRACE("chn:%u,bits:%u,rate:%u", m_info.atinfo.channels, m_info.atinfo.sample_bits*8, m_info.atinfo.sample_rate);
}

CMediaTrackAAC::~CMediaTrackAAC() {}

bool CMediaTrackAAC::Init(void *pinfo) {
#if 0
    static const char *pattern =
        "m=audio 0 RTP/AVP %d\n"
        "a=rtpmap:%d MPEG4-GENERIC/%d/%d\n"
        "a=fmtp:%d "
        "streamType=5;profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config=";
#endif
    uint32_t ssrc = rtp_ssrc();
    m_timestamp = ssrc;
    static struct rtp_payload_t s_rtpfunc = {
        CMediaTrack::RTPAlloc,
        CMediaTrack::RTPFree,
        CMediaTrack::RTPPacket,
    };

    struct rtp_event_t event;

    event.on_rtcp = CMediaTrack::OnRTCPEvent;

    m_fiforeader = new CShmStreamR(m_info.fifosize, m_info.name, m_info.chn);
    if (!m_fiforeader) {
        LOG_ERROR("Create m_fiforeader");
        goto _err;
    }

    if (!m_fiforeader->ShmAlloc()) {
        LOG_ERROR("ShmAlloc error");
        goto _err;
    }

    m_rtppacker = rtp_payload_encode_create(RTP_PAYLOAD_MP4A, "mpeg4-generic", (uint16_t)ssrc, ssrc, &s_rtpfunc, this);
    if (!m_rtppacker) {
        LOG_ERROR("Create playload encode error AAC");
        goto _err;
    }

    m_rtp = rtp_create(&event, this, ssrc, ssrc, m_info.atinfo.sample_rate, ZC_AUDIO_BANDWIDTH, 1);
    if (!m_rtp) {
        LOG_ERROR("Create video track error AAC");
        goto _err;
    }

    rtp_set_info(m_rtp, "RTSPServer", "live.aac");
    // set create flag
    m_create = true;
    LOG_TRACE("Create ok ,AAC");
    return true;
_err:
    LOG_ERROR("Create error AAC");
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
}  // namespace zc
