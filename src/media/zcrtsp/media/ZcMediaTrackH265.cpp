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

#include "zc_base64.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcMediaTrackH265.hpp"
#include "ZcType.hpp"

extern "C" uint32_t rtp_ssrc(void);

#define VIDEO_BANDWIDTH (4 * 1024)
#define VIDEO_FREQUENCE (90000)  // frequence
#define ZC_RTP_PAYLOAD_H265 98   // H.265 video (MPEG-H Part 2) (rfc7798)
namespace zc {
CMediaTrackH265::CMediaTrackH265(const zc_meida_track_t &info) : CMediaTrack(info) {
    m_frequency = VIDEO_FREQUENCE;
    LOG_TRACE("Constructor");
}

CMediaTrackH265::~CMediaTrackH265() {}

bool CMediaTrackH265::Init(void *pinfo) {
    LOG_TRACE("Create H265 into");
    char sdpbuf[1024];
    char spspps[1024] = {0};
    int payload = ZC_RTP_PAYLOAD_H265;  // RTP_PAYLOAD_H265
    size_t spslen = 0;
    zc_frame_userinfo_t frameinfo = {0};
    uint32_t ssrc = rtp_ssrc();
    m_timestamp = ssrc;
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
        "a=rtpmap:%d H265/%d\n"
        "a=fmtp:%d profile-level-id=%02X%02X%02X;packetization-mode=1;sprop-parameter-sets=%s\n"
        "a=control:track%d\n";

    const char *test_sps = "QAEMAf//AWAAAAMAsAAAAwAAAwBdqgJAAAAAAQ==";
    // profile-level-id=010C01;
    char profileid[3] = {0x01, 0x0C, 0x01};

    m_fiforeader = new CShmStreamR(m_info.fifosize, m_info.name, m_info.chn);
    if (!m_fiforeader) {
        LOG_ERROR("Create m_fiforeader");
        goto _err;
    }

    if (!m_fiforeader->ShmAlloc()) {
        LOG_ERROR("ShmAlloc error");
        goto _err;
    }

    if (m_fiforeader->GetStreamInfo(frameinfo, true)) {
        LOG_ERROR("GetStreamInfo ok");
        // goto _err;
        for (int i = 0; i < frameinfo.vinfo.nalunum; i++) {
            // sps pps
            if (frameinfo.vinfo.nalu[i].type >= ZC_NALU_TYPE_VPS && frameinfo.vinfo.nalu[i].type <= ZC_NALU_TYPE_PPS) {
                // sps - pfofileid
                if (i == 0) {
                    profileid[0] = frameinfo.vinfo.nalu[0].data[1];
                    profileid[1] = frameinfo.vinfo.nalu[0].data[2];
                    profileid[2] = frameinfo.vinfo.nalu[0].data[3];
                } else if (spslen > 0) {
                    spspps[spslen++] = ',';
                }

                spslen += zc_base64_encode(spspps + spslen, frameinfo.vinfo.nalu[i].data, frameinfo.vinfo.nalu[i].size);
            }
        }
        // end
        spspps[spslen++] = '\0';
        LOG_ERROR("GetStreamInfo spspps[%s]", spspps);
    }

    m_rtppacker = rtp_payload_encode_create(payload, "h265", (uint16_t)ssrc, ssrc, &s_rtpfunc, this);
    if (!m_rtppacker) {
        LOG_ERROR("Create playload encode error H265");
        goto _err;
    }

    m_rtp = rtp_create(&event, this, ssrc, ssrc, m_frequency, VIDEO_BANDWIDTH, 1);
    if (!m_rtp) {
        LOG_ERROR("Create video track error H265");
        goto _err;
    }

    rtp_set_info(m_rtp, "RTSPServer", "live.h265");

    // sps
    snprintf(sdpbuf, sizeof(sdpbuf), video_pattern, payload, payload, m_frequency, payload,
             profileid[0], profileid[1], profileid[2], spspps, m_info.trackno);
    LOG_TRACE("ok H265 sdp sdpbuf[%s]", sdpbuf);
    m_sdp = sdpbuf;
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
