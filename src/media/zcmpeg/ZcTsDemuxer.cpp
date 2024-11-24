// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "mpeg-ps.h"
#include "mpeg-ts.h"
#include "mpeg-util.h"
#include "zc_basic_fun.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_proc.h"

#include "Epoll.hpp"
#include "Thread.hpp"
#include "ZcStreamTrace.hpp"
#include "ZcTsDemuxer.hpp"
#include "ZcType.hpp"

namespace zc {

static inline size_t get_adts_length(const uint8_t *data, size_t bytes) {
    assert(bytes >= 6);
    return ((data[3] & 0x03) << 11) | (data[4] << 3) | ((data[5] >> 5) & 0x07);
}

int CTsDemuxer::onTsCb(void *ptr, int program, int stream, int codec, int flags, int64_t pts, int64_t dts,
                       const void *data, size_t bytes) {
    return reinterpret_cast<CTsDemuxer *>(ptr)->_onTsCb(program, stream, codec, flags, pts, dts, data, bytes);
}

int CTsDemuxer::_onTsCb(int program, int stream, int codec, int flags, int64_t pts, int64_t dts, const void *data,
                        size_t bytes) {
    int ret = 0;
    zc_frame_t framehdr = {0};
    if (PSI_STREAM_H264 == codec || PSI_STREAM_H265 == codec || PSI_STREAM_H266 == codec ||
        PSI_STREAM_VIDEO_AVS3 == codec) {
        if (flags) {
            LOG_TRACE("video codec:%d,flags:%d, bytes:%d, pts:%u, diff: %03d/%03d %s", codec, flags, bytes, pts,
                      (int)(pts - dframeinfo[ZC_STREAM_VIDEO].pts), (int)(dts - dframeinfo[ZC_STREAM_VIDEO].dts),
                      flags ? "[I]" : "");
        }
        // m_demuxerouttrace.Write(data, bytes);
        // fwrite(data, bytes, 1, h264);
        framehdr.keyflag = flags ? 1 : 0;
        framehdr.video.encode = ZC_FRAME_ENC_H264;
        framehdr.type = ZC_STREAM_VIDEO;
        framehdr.size = bytes;
        framehdr.pts = pts/90;
        framehdr.utc = zc_system_time();
        dframeinfo[ZC_STREAM_VIDEO].pts = pts;
        dframeinfo[ZC_STREAM_VIDEO].dts = dts;
        framehdr.seq = dframeinfo[ZC_STREAM_VIDEO].seqno++;
        if (PSI_STREAM_H265 == codec) {
            framehdr.video.encode = ZC_FRAME_ENC_H265;
        }
    } else if (PSI_STREAM_AAC == codec || PSI_STREAM_AUDIO_OPUS == codec) {
        // LOG_TRACE("audio aac bytes:%d, pts:%u,diff:%03d/%03d", bytes, pts, (int)(pts -
        // dframeinfo[ZC_STREAM_AUDIO].pts),
        //           (int)(dts - dframeinfo[ZC_STREAM_AUDIO].dts));

        assert(bytes == get_adts_length((const uint8_t *)data, bytes));
        framehdr.audio.encode = ZC_FRAME_ENC_AAC;
        framehdr.type = ZC_STREAM_AUDIO;
        framehdr.size = bytes;
        framehdr.pts = pts/90;
        framehdr.utc = zc_system_time();
        dframeinfo[ZC_STREAM_AUDIO].pts = pts;
        dframeinfo[ZC_STREAM_AUDIO].dts = dts;
        framehdr.seq = dframeinfo[ZC_STREAM_AUDIO].seqno++;
    } else {
        LOG_WARN("unsupport codec:%d, bytes:%u", codec, bytes);
        // nothing to do
    }

    if (framehdr.size > 0 && m_info.onframe) {
        m_info.onframe(m_info.ctx, &framehdr, (const uint8_t *)data);
    }
    return ret;
}

CTsDemuxer::CTsDemuxer(const zc_tsdemuxer_info_t &cb) {
    memcpy(&m_info, &cb, sizeof(zc_tsdemuxer_info_t));
    // m_demuxerouttrace.Open("./tsdemuxerout_trace.ts", "wb");
    // m_demuxerintrace.Open("./tsdemuxerin_trace.ts", "wb");

    m_ts = ts_demuxer_create(onTsCb, this);
    ZC_ASSERT(m_ts != nullptr);
}

CTsDemuxer::~CTsDemuxer() {
    if (m_ts) {
        ts_demuxer_destroy(reinterpret_cast<ts_demuxer_t *>(m_ts));
    }
}

int CTsDemuxer::Input(const uint8_t *data, size_t bytes) {
    ZC_ASSERT(m_ts != nullptr);
    int ret = 0;
    const uint8_t *packet = data;
    // LOG_WARN("Input bytes:%u", bytes);
    while (bytes >= 188) {
        ret = ts_demuxer_input(reinterpret_cast<ts_demuxer_t *>(m_ts), packet, 188);
        if (0 != ret) {
            ZC_ASSERT(0 == ret);
            break;
        }
        // m_demuxerintrace.Write(packet, 188);
        bytes -= 188;
        packet += 188;
    }

    return 0;
}

}  // namespace zc
