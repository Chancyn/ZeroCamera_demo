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

#include "zc_log.h"
#include "zc_macros.h"
#include "zc_media_fifo_def.h"
#include "zc_type.h"

#include "ZcMediaTrackH265.hpp"
#include "ZcType.hpp"

extern "C" uint32_t rtp_ssrc(void);

#define VIDEO_BANDWIDTH (4 * 1024)
#define VIDEO_FREQUENCE (90000)  // frequence

namespace zc {
CMediaTrackH265::CMediaTrackH265() : CMediaTrack(MEDIA_TRACK_VIDEO, MEDIA_CODE_H265) {}

CMediaTrackH265::~CMediaTrackH265() {}

bool CMediaTrackH265::Init(void *pinfo) {
    uint32_t ssrc = rtp_ssrc();
    static struct rtp_payload_t s_rtpfunc = {
        CMediaTrack::RTPAlloc,
        CMediaTrack::RTPFree,
        CMediaTrack::RTPPacket,
    };

    struct rtp_event_t event;

    event.on_rtcp = CMediaTrack::OnRTCPEvent;

    // video
    static const char *video_pattern =
        "m=video 0 RTP/AVP %d\n"
        "a=rtpmap:%d H265/90000\n"
        "a=fmtp:%d profile-level-id=%02X%02X%02X;packetization-mode=1;sprop-parameter-sets=";

    m_fiforeader = new CShmFIFOR(ZC_MEDIA_MAIN_VIDEO_SIZE, ZC_MEDIA_VIDEO_SHM_PATH, 0);
    if (!m_fiforeader) {
        LOG_ERROR("Create m_fiforeader");
        goto _err;
    }

    m_rtppacker = rtp_payload_encode_create(RTP_PAYLOAD_H265, "h265", (uint16_t)ssrc, ssrc, &s_rtpfunc, this);
    if (!m_rtppacker) {
        LOG_ERROR("Create playload encode error H265");
        goto _err;
    }

    m_rtp = rtp_create(&event, this, ssrc, ssrc, VIDEO_FREQUENCE, VIDEO_BANDWIDTH, 1);
    if (!m_rtp) {
        LOG_ERROR("Create video track error H265");
        goto _err;
    }

    rtp_set_info(m_rtp, "RTSPServer", "live.h265");
    // set create flag
    m_create = true;
    LOG_TRACE("Create ok H265");
    return true;
_err:
    LOG_ERROR("Create error H265");
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