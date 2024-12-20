// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "flv-muxer.h"
#include "flv-proto.h"
#include "flv-reader.h"
#include "zc_macros.h"
#include "zc_basic_fun.h"
#include "flv-demuxer.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_proc.h"

#include "Epoll.hpp"
#include "Thread.hpp"
#include "ZcFlvDemuxer.hpp"
#include "ZcStreamTrace.hpp"
#include "ZcType.hpp"

namespace zc {

static inline size_t get_adts_length(const uint8_t *data, size_t bytes) {
    assert(bytes >= 6);
    return ((data[3] & 0x03) << 11) | (data[4] << 3) | ((data[5] >> 5) & 0x07);
}

int CFlvDemuxer::onFlvCb(void *ptr, int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags) {
    return reinterpret_cast<CFlvDemuxer *>(ptr)->_onFlvCb(codec, data, bytes, pts, dts, flags);
}

int CFlvDemuxer::_onFlvCb(int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags) {
    int ret = 0;
    zc_frame_t framehdr = {0};
    if (FLV_VIDEO_H264 == codec || FLV_VIDEO_H265 == codec || FLV_VIDEO_H266 == codec || FLV_VIDEO_AV1 == codec) {
        // if (flags) {
        //     LOG_TRACE("video codec:%d,flags:%d, bytes:%d, pts:%u, diff: %03d/%03d %s", codec, flags, bytes, pts,
        //               (int)(pts - dframeinfo[ZC_STREAM_VIDEO].pts), (int)(dts - dframeinfo[ZC_STREAM_VIDEO].dts),
        //               flags ? "[I]" : "");
        // }

        // fwrite(data, bytes, 1, h264);
        framehdr.keyflag = flags ? 1 : 0;
        framehdr.video.encode = ZC_FRAME_ENC_H264;
        framehdr.type = ZC_STREAM_VIDEO;
        framehdr.size = bytes;
        framehdr.pts = pts;
        framehdr.utc = zc_system_time();
        dframeinfo[ZC_STREAM_VIDEO].pts = pts;
        dframeinfo[ZC_STREAM_VIDEO].dts = dts;
        framehdr.seq = dframeinfo[ZC_STREAM_VIDEO].seqno++;
        if (FLV_VIDEO_H265 == codec) {
            framehdr.video.encode = ZC_FRAME_ENC_H265;
        }
    } else if (FLV_AUDIO_AAC == codec) {
        // LOG_TRACE("audio aac bytes:%d, pts:%u,diff:%03d/%03d", bytes, pts, (int)(pts - dframeinfo[ZC_STREAM_AUDIO].pts),
        //           (int)(dts - dframeinfo[ZC_STREAM_AUDIO].dts));
        assert(bytes == get_adts_length((const uint8_t *)data, bytes));
        framehdr.audio.encode = ZC_FRAME_ENC_AAC;
        framehdr.type = ZC_STREAM_AUDIO;
        framehdr.size = bytes;
        framehdr.pts = pts;
        framehdr.utc = zc_system_time();
        dframeinfo[ZC_STREAM_AUDIO].pts = pts;
        dframeinfo[ZC_STREAM_AUDIO].dts = dts;
        framehdr.seq = dframeinfo[ZC_STREAM_AUDIO].seqno++;
        // fwrite(data, bytes, 1, aac);
    } else if (FLV_AUDIO_MP3 == codec || FLV_AUDIO_OPUS == codec) {
        LOG_TRACE("audio codec:%d, bytes:%d, pts:%u,diff:%03d/%03d", codec, bytes, pts,
                  (int)(pts - dframeinfo[ZC_STREAM_AUDIO].pts), (int)(dts - dframeinfo[ZC_STREAM_AUDIO].dts));
        framehdr.audio.encode = ZC_FRAME_ENC_AAC;
        framehdr.type = ZC_STREAM_AUDIO;
        framehdr.size = bytes;
        framehdr.pts = pts;
        framehdr.utc = zc_system_time();
        dframeinfo[ZC_STREAM_AUDIO].pts = pts;
        dframeinfo[ZC_STREAM_AUDIO].dts = dts;
        framehdr.seq = dframeinfo[ZC_STREAM_AUDIO].seqno++;
        // fwrite(data, bytes, 1, aac);
    } else if (FLV_AUDIO_ASC == codec || FLV_AUDIO_OPUS_HEAD == codec || FLV_VIDEO_AVCC == codec ||
               FLV_VIDEO_HVCC == codec || FLV_VIDEO_VVCC == codec || FLV_VIDEO_AV1C == codec) {
        // nothing to do
        LOG_TRACE("unsupport codec:%d, bytes:%u", codec, bytes);
    } else if ((3 << 4) == codec) {
        // fwrite(data, bytes, 1, aac);
    } else {
        LOG_WARN("unsupport codec:%d, bytes:%u", codec, bytes);
        // nothing to do
        assert(FLV_SCRIPT_METADATA == codec);
    }

    if (framehdr.size > 0 && m_info.onframe) {
        m_info.onframe(m_info.ctx, &framehdr, (const uint8_t *)data);
    }
    return ret;
}

CFlvDemuxer::CFlvDemuxer(const zc_flvdemuxer_info_t &cb) {
    memcpy(&m_info, &cb, sizeof(zc_flvdemuxer_info_t));
    m_flv = flv_demuxer_create(onFlvCb, this);
    ZC_ASSERT(m_flv != nullptr);
}

CFlvDemuxer::~CFlvDemuxer() {
    if (m_flv) {
        flv_demuxer_destroy(reinterpret_cast<flv_demuxer_t *>(m_flv));
    }
}

int CFlvDemuxer::Input(int type, const uint8_t *data, size_t bytes, uint32_t timestamp) {
    ZC_ASSERT(m_flv != nullptr);
    return flv_demuxer_input(reinterpret_cast<flv_demuxer_t *>(m_flv), type, data, bytes, timestamp);
}

}  // namespace zc
