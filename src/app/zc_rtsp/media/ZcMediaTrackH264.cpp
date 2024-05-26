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
#include "zc_frame.h"
#include "zc_type.h"

#include "ZcMediaTrackH264.hpp"
#include "ZcType.hpp"

extern "C" uint32_t rtp_ssrc(void);

#define VIDEO_BANDWIDTH (4 * 1024)
#define VIDEO_FREQUENCE (90000)  // frequence

namespace zc {
CMediaTrackH264::CMediaTrackH264() : CMediaTrack(MEDIA_TRACK_VIDEO, MEDIA_CODE_H264) {
    LOG_TRACE("Constructor");
}

CMediaTrackH264::~CMediaTrackH264() {}

bool CMediaTrackH264::Init(void *pinfo) {
    LOG_TRACE("Create H264 into");
    char sdpbuf[1024];
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
        "a=rtpmap:%d H264/%d\n"
        "a=fmtp:%d profile-level-id=%02X%02X%02X;packetization-mode=1;sprop-parameter-sets=%s\n";

    const char *test_sps = "Z00AKpY1QPAET8s3AQEBAgAAAAE=,aO4xsgAAAAEG5QHpgAAAAAFluAAADJ1wAAE/6Q==";
    // profile-level-id=4D002A;
    char profileid[3] = {0x4D, 0x00, 0x2A};
    //m_fiforeader = new CShmFIFOR(ZC_STREAM_MAIN_VIDEO_SIZE, ZC_STREAM_VIDEO_SHM_PATH, 0);
    m_fiforeader = new CShmStreamR(ZC_STREAM_SUB_VIDEO_SIZE, ZC_STREAM_VIDEO_SHM_PATH, 1);
    if (!m_fiforeader) {
        LOG_ERROR("Create m_fiforeader error");
        goto _err;
    }

    if (!m_fiforeader->ShmAlloc()) {
        LOG_ERROR("ShmAlloc error");
        goto _err;
    }

    m_rtppacker = rtp_payload_encode_create(RTP_PAYLOAD_H264, "h264", (uint16_t)ssrc, ssrc, &s_rtpfunc, this);
    if (!m_rtppacker) {
        LOG_ERROR("Create playload encode error H264");
        goto _err;
    }

    m_rtp = rtp_create(&event, this, ssrc, ssrc, VIDEO_FREQUENCE, VIDEO_BANDWIDTH, 1);
    if (!m_rtp) {
        LOG_ERROR("Create video track error H264");
        goto _err;
    }

    rtp_set_info(m_rtp, "RTSPServer", "live.h264");

    // sps
    snprintf(sdpbuf, sizeof(sdpbuf), video_pattern, RTP_PAYLOAD_H264, RTP_PAYLOAD_H264, VIDEO_FREQUENCE,
             RTP_PAYLOAD_H264, profileid[0], profileid[1], profileid[2], test_sps);
    LOG_TRACE("ok H264 sdp sdpbuf[%s]", sdpbuf);
    m_sdp = sdpbuf;
    // set create flag
    m_create = true;
    LOG_TRACE("Create ok H264");
    return true;
_err:
    LOG_ERROR("Create error H264");
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
