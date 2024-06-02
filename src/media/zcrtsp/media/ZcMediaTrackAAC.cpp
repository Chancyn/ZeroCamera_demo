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

extern "C" uint32_t rtp_ssrc(void);

#define AUDIO_BANDWIDTH (4 * 1024)  // bandwidth
#define AUDIO_FREQUENCE (48000)     // frequence

namespace zc {
CMediaTrackAAC::CMediaTrackAAC(int shmtype, int chn)
    : CMediaTrack(ZC_MEDIA_TRACK_AUDIO, ZC_FRAME_ENC_AAC, ZC_MEDIA_CODE_AAC, shmtype, chn) {
    memset(&m_meidainfo, 0, sizeof(m_meidainfo));
    m_meidainfo.channels = 2;
    m_meidainfo.sample_bits = 2;
    m_meidainfo.sample_rate = AUDIO_FREQUENCE;
    m_frequency = AUDIO_FREQUENCE;
}

CMediaTrackAAC::~CMediaTrackAAC() {}

bool CMediaTrackAAC::Init(void *pinfo) {
    audio_info_t info;
    if (pinfo) {
        memcpy(&info, pinfo, sizeof(audio_info_t));
        LOG_WARN("user audio_info chn[%d], bits[%d], rate[%d]", info.channels, info.sample_bits, info.sample_rate);
    } else {
        memcpy(&info, &m_meidainfo, sizeof(audio_info_t));
    }
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

    // m_fiforeader = new CShmFIFOR(ZC_STREAM_AUDIO_SIZE, ZC_STREAM_AUDIO_SHM_PATH, 0);
    if (m_shmtype == ZC_SHMSTREAM_PUSH) {
        // push stream
        m_fiforeader = new CShmStreamR(ZC_STREAM_AUDIO_SIZE, ZC_STREAM_VIDEOPUSH_SHM_PATH, m_chn);
    } else if (m_shmtype == ZC_SHMSTREAM_PULL) {
        // pull stream
        m_fiforeader = new CShmStreamR(ZC_STREAM_AUDIO_SIZE, ZC_STREAM_VIDEOPULL_SHM_PATH, m_chn);
    } else {
        // live stream
        m_fiforeader = new CShmStreamR(ZC_STREAM_AUDIO_SIZE, ZC_STREAM_AUDIO_SHM_PATH, m_chn);
    }

    if (!m_fiforeader) {
        LOG_ERROR("Create m_fiforeader");
        goto _err;
    }

    m_rtppacker = rtp_payload_encode_create(RTP_PAYLOAD_MP4A, "mpeg4-generic", (uint16_t)ssrc, ssrc, &s_rtpfunc, this);
    if (!m_rtppacker) {
        LOG_ERROR("Create playload encode error AAC");
        goto _err;
    }

    m_rtp = rtp_create(&event, this, ssrc, ssrc, m_meidainfo.sample_rate, AUDIO_BANDWIDTH, 1);
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
