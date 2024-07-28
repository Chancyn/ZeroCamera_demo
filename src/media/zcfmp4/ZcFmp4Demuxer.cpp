// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/syslog.h>
#include <sys/types.h>
#include <unistd.h>

#include "aom-av1.h"
#include "mov-format.h"
#include "mov-reader.h"
#include "mpeg4-aac.h"
#include "mpeg4-avc.h"
#include "mpeg4-hevc.h"
#include "opus-head.h"
#include "webm-vpx.h"
#include "zc_h26x_sps_parse.h"
// #include "sys/system.h"
#include "zc_frame.h"
#include "zc_log.h"

#include "ZcFmp4Demuxer.hpp"
#include "ZcType.hpp"
#include "zc_macros.h"

namespace zc {
#define ZC_FMP4DE_DEBUG_DUMP 1
#define USE_NEW_MOV_READ_API 1

#if 0
static struct mpeg4_hevc_t s_hevc;
static struct mpeg4_avc_t s_avc;
static struct mpeg4_aac_t s_aac;
static struct webm_vpx_t s_vpx;
static struct opus_head_t s_opus;
static struct aom_av1_t s_av1;
#endif

inline const char *ftimestamp(uint32_t t, char *buf, unsigned int len) {
    snprintf(buf, len, "%02u:%02u:%02u.%03u", t / 3600000, (t / 60000) % 60, (t / 1000) % 60, t % 1000);
    return buf;
}

CFmp4DeMuxer::CFmp4DeMuxer()
    : Thread("mp4demuxer"), m_open(0), m_status(fmp4de_status_init), m_bufferlen(ZC_FMP4_RBUF_MAX),
      m_annexbbuflen(ZC_FMP4_RBUF_ANNEXB_MAX), m_buffer(nullptr), m_annexbbuf(nullptr), m_movbuf(nullptr),
      m_reader(nullptr), m_duration(0), m_mpeg4video(nullptr), m_mpeg4audio(nullptr) {
    memset(&m_pkt, 0, sizeof(m_pkt));
    memset(&m_lastts, 0, sizeof(m_lastts));

    m_tracks.clear();
}

CFmp4DeMuxer::~CFmp4DeMuxer() {
    Close();
}

void CFmp4DeMuxer::OnMovVideoInfo(void *param, uint32_t track, uint8_t object, int width, int height, const void *extra,
                                  size_t bytes) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onMovVideoInfo(track, object, width, height, extra, bytes);
}

void CFmp4DeMuxer::_onMovVideoInfo(uint32_t track, uint8_t object, int width, int height, const void *extra,
                                   size_t bytes) {
    if (MOV_OBJECT_H264 == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_vfp = fopen("v.h264", "wb");
#endif
        m_tracks[track] = "H264";
        struct mpeg4_avc_t *avc = new struct mpeg4_avc_t();
        m_mpeg4video = avc;
        ZC_ASSERT(m_mpeg4video);
        mpeg4_avc_decoder_configuration_record_load((const uint8_t *)extra, bytes, avc);
    } else if (MOV_OBJECT_HEVC == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_vfp = fopen("v.h265", "wb");
#endif
        m_tracks[track] = "H265";
        struct mpeg4_hevc_t *hevc = new struct mpeg4_hevc_t();
        m_mpeg4video = hevc;
        ZC_ASSERT(m_mpeg4video);
        mpeg4_hevc_decoder_configuration_record_load((const uint8_t *)extra, bytes, hevc);
    }
#if 0
    else if (MOV_OBJECT_AV1 == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_vfp = fopen("v.obus", "wb");
#endif
        m_tracks[track] = "AV1";
        struct aom_av1_t *av1 = new struct aom_av1_t();
        m_mpeg4video = av1;
        ZC_ASSERT(m_mpeg4video);
        aom_av1_codec_configuration_record_load((const uint8_t *)extra, bytes, av1);
    } else if (MOV_OBJECT_VP9 == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_vfp = fopen("v.vp9", "wb");
#endif
        m_tracks[track] = "VP9";
        struct webm_vpx_t *vpx = new struct webm_vpx_t();
        m_mpeg4video = vpx;
        ZC_ASSERT(m_mpeg4video);
        webm_vpx_codec_configuration_record_load((const uint8_t *)extra, bytes, vpx);
    }
#endif
    else {
        LOG_WARN("unsupport video encode:%u", object);
        m_tracks[track] = "VIDEO";
    }

    return;
}

void CFmp4DeMuxer::OnMovAudioInfo(void *param, uint32_t track, uint8_t object, int channel_count, int bit_per_sample,
                                  int sample_rate, const void *extra, size_t bytes) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onMovAudioInfo(track, object, channel_count, bit_per_sample,
                                                                   sample_rate, extra, bytes);
}

void CFmp4DeMuxer::_onMovAudioInfo(uint32_t track, uint8_t object, int channel_count, int bit_per_sample,
                                   int sample_rate, const void *extra, size_t bytes) {
    if (MOV_OBJECT_AAC == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.aac", "wb");
#endif
        m_tracks[track] = "AAC";
        struct mpeg4_aac_t *aac = new struct mpeg4_aac_t();
        m_mpeg4audio = aac;
        ZC_ASSERT(m_mpeg4audio);
        ZC_ASSERT(bytes == mpeg4_aac_audio_specific_config_load((const uint8_t *)extra, bytes, aac));
        ZC_ASSERT(channel_count == aac->channels);
        ZC_ASSERT(MOV_OBJECT_AAC == object);
        aac->profile = MPEG4_AAC_LC;
        aac->channel_configuration = channel_count;
        aac->sampling_frequency_index = mpeg4_aac_audio_frequency_from(sample_rate);
    }
#if 0
    else if (MOV_OBJECT_OPUS == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.opus", "wb");
#endif
        m_tracks[track] = "OPUS";
        struct opus_head_t *opus = new struct opus_head_t();
        m_mpeg4audio = opus;
        ZC_ASSERT(m_mpeg4audio);
        ZC_ASSERT(bytes == opus_head_load((const uint8_t *)extra, bytes, opus));
        ZC_ASSERT(opus->input_sample_rate == 48000);
    } else if (MOV_OBJECT_MP3 == object || MOV_OBJECT_MP1A == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.mp3", "wb");
#endif
        m_tracks[track] = "MP3";
    } else if (MOV_OBJECT_G711a == object || MOV_OBJECT_G711u == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.raw", "wb");
#endif
        m_tracks[track] = "G711";
    }
#endif
    else {
        LOG_WARN("unsupport audio encode:%u", object);
        // s_aac_track = track;
        // s_aac.channel_configuration = channel_count;
        // s_aac.sampling_frequency_index = mpeg4_aac_audio_frequency_from(sample_rate);
        m_tracks[track] = "AUDIO";
    }

    return;
}

void CFmp4DeMuxer::OnMovSubtitleInfo(void *param, uint32_t track, uint8_t object, const void *extra, size_t bytes) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onMovSubtitleInfo(track, object, extra, bytes);
}

void CFmp4DeMuxer::_onMovSubtitleInfo(uint32_t track, uint8_t object, const void *extra, size_t bytes) {
    m_tracks[track] = "SUBTITLE";
    return;
}

bool CFmp4DeMuxer::Open(const char *name) {
    if (m_open) {
        return false;
    }

    m_movbuf = new CMovReadIo(name);
    if (!m_movbuf) {
        return false;
    }

    struct mov_reader_trackinfo_t info = {
        .onvideo = OnMovVideoInfo,
        .onaudio = OnMovAudioInfo,
        .onsubtitle = OnMovSubtitleInfo,
    };

    m_buffer = new uint8_t[m_bufferlen];
    if (!m_buffer) {
        LOG_ERROR("reader new buf error");
        goto _err;
    }

    m_annexbbuf = new uint8_t[m_annexbbuflen];
    if (!m_annexbbuf) {
        LOG_ERROR("reader new annexb buf error");
        goto _err;
    }
    m_reader = mov_reader_create(m_movbuf->GetIoInfo(), m_movbuf);
    if (!m_reader) {
        LOG_ERROR("reader create error");
        goto _err;
    }

    m_duration = mov_reader_getduration(m_reader);

    if (mov_reader_getinfo(m_reader, &info, this) < 0) {
        LOG_ERROR("reader create error");
        goto _err;
    }

    m_name = name;
    m_open = 1;
    LOG_TRACE("Open ok:%s", m_name.c_str());
    return true;
_err:
    mov_reader_destroy(m_reader);
    m_reader = nullptr;
    ZC_SAFE_DELETEA(m_annexbbuf);
    ZC_SAFE_DELETEA(m_buffer);
    ZC_SAFE_DELETE(m_movbuf);
    LOG_TRACE("Open error:%s", name);
    return false;
}

bool CFmp4DeMuxer::Close() {
    if (!m_open) {
        return false;
    }

    mov_reader_destroy(m_reader);
    m_reader = nullptr;
    ZC_SAFE_DELETEA(m_annexbbuf);
    ZC_SAFE_DELETEA(m_buffer);
    ZC_SAFE_DELETE(m_movbuf);
    m_name.clear();
    m_open = 0;
    LOG_TRACE("Open ok");
    return true;
}

bool CFmp4DeMuxer::Start() {
    if (!m_open) {
        return false;
    }

    // find idr;

    Thread::Start();
    m_status = fmp4de_status_err;
    return true;
}

bool CFmp4DeMuxer::Stop() {
    Thread::Stop();
    return false;
}

void CFmp4DeMuxer::OnRead(void *param, uint32_t track, const void *buffer, size_t bytes, int64_t pts, int64_t dts,
                          int flags) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onRead(track, buffer, bytes, pts, dts, flags);
}

void CFmp4DeMuxer::_onRead(uint32_t track, const void *buffer, size_t bytes, int64_t pts, int64_t dts, int flags) {

    auto it = m_tracks.find(track);
    if (it == m_tracks.end()) {
        assert(0);
        return;
    }

    if (it->second == "H264") {
        LOG_WARN("[H264] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u%s", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (int)(pts - m_lastts.v_pts), (int)(dts - m_lastts.v_dts), (unsigned int)bytes,
                 flags ? " [I]" : "");
        m_lastts.v_pts = pts;
        m_lastts.v_dts = dts;

        assert(h264_is_new_access_unit((const uint8_t *)buffer + 4, bytes - 4));
        int n = h264_mp4toannexb((const struct mpeg4_avc_t* )m_mpeg4video, buffer, bytes, m_annexbbuf, sizeof(m_annexbbuf));
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        fwrite(m_annexbbuf, 1, n, m_debug_vfp);
#endif
    } else if (it->second == "H265") {
        uint8_t nalu_type = (((const uint8_t *)buffer)[4] >> 1) & 0x3F;
        uint8_t irap = 16 <= nalu_type && nalu_type <= 23;

        LOG_WARN("[H265] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u%s,%d", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts - m_lastts.v_pts), (dts - m_lastts.v_dts), (unsigned int)bytes,
                 flags ? " [I]" : "", (unsigned int)nalu_type);
        m_lastts.v_pts = pts;
        m_lastts.v_dts = dts;

        assert(h265_is_new_access_unit((const uint8_t *)buffer + 4, bytes - 4));
        int n = h265_mp4toannexb((const struct mpeg4_hevc_t* )m_mpeg4video, buffer, bytes, m_annexbbuf, sizeof(m_annexbbuf));
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        fwrite(m_annexbbuf, 1, n, m_debug_vfp);
#endif
    } else if (it->second == "AV1") {
        LOG_WARN("[AV1] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u%s", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts - m_lastts.v_pts), (dts - m_lastts.v_dts), (unsigned int)bytes,
                 flags ? " [I]" : "");
        m_lastts.v_pts = pts;
        m_lastts.v_dts = dts;
        // TODO(zhoucc): test
        // int n = aom_av1_codec_configuration_record_save((const struct aom_av1_t* )m_mpeg4video, m_annexbbuf, sizeof(m_annexbbuf));
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        // fwrite(m_annexbbuf, 1, n, m_debug_vfp);
#endif
    } else if (it->second == "VPX") {
        LOG_WARN("[VPX] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u%s", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts - m_lastts.v_pts), (dts - m_lastts.v_dts), (unsigned int)bytes,
                 flags ? " [I]" : "");
        m_lastts.v_pts = pts;
        m_lastts.v_dts = dts;
        // TODO(zhoucc): test
       // int n = webm_vpx_codec_configuration_record_save((const struct webm_vpx_t* )m_mpeg4video, m_annexbbuf, sizeof(m_annexbbuf));
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        // fwrite(m_annexbbuf, 1, n, m_debug_vfp);
#endif
    } else if (it->second == "AAC") {
        LOG_WARN("[AAC] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts - m_lastts.a_pts), (dts - m_lastts.a_dts), (unsigned int)bytes);
        m_lastts.a_pts = pts;
        m_lastts.a_dts = dts;

        uint8_t adts[32];
        int n = mpeg4_aac_adts_save((const struct mpeg4_aac_t* )m_mpeg4audio, bytes, adts, sizeof(adts));
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        fwrite(adts, 1, n, m_debug_afp);
        fwrite(buffer, 1, bytes, m_debug_afp);
#endif
    }
#if 0
    else if (it->second == "OPUS") {
        LOG_WARN("[OPUS] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts - m_lastts.a_pts), (dts - m_lastts.a_dts), (unsigned int)bytes);
        m_lastts.a_pts = pts;
        m_lastts.a_dts = dts;
    } else if (it->second == "MP3") {
        LOG_WARN("[MP3] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts - m_lastts.a_pts), (dts - m_lastts.a_dts), (unsigned int)bytes);
        m_lastts.a_pts = pts;
        m_lastts.a_dts = dts;
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        fwrite(buffer, 1, bytes, m_debug_afp);
#endif
    } else if (it->second == "G711") {
        // static int64_t t_pts, t_dts;
        LOG_WARN("[G711] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u", ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                 ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts -  m_lastts.a_pts), (dts - m_lastts.a_dts), (unsigned int)bytes);
        m_lastts.a_pts = pts;
        m_lastts.a_dts = dts;
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        fwrite(buffer, 1, bytes, m_debug_afp);
#endif
    }
#endif
    else {
        LOG_WARN("[%s] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u", it->second.c_str(),
                 ftimestamp(pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)), ftimestamp(dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pts - m_lastts.x_pts),
                 (dts - m_lastts.x_dts), (unsigned int)bytes);
        m_lastts.x_pts = pts;
        m_lastts.x_dts = dts;
        // assert(0);
    }
}

void *CFmp4DeMuxer::OnAlloc(void *param, uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onAlloc(track, bytes, pts, dts, flags);
}

void *CFmp4DeMuxer::_onAlloc(uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags) {
    // emulate allocation
    if (m_pkt.bytes < bytes)
        return nullptr;

    m_pkt.flags = flags;
    m_pkt.pts = pts;
    m_pkt.dts = dts;
    m_pkt.track = track;
    m_pkt.bytes = bytes;
    return m_pkt.ptr;
}

int CFmp4DeMuxer::readerProcess() {
    int ret = 0;
    while (State() == Running) {
        m_pkt.ptr = m_buffer;
        m_pkt.bytes = sizeof(m_buffer);
        ret = mov_reader_read2(m_reader, OnAlloc, this);
        if (ret <= 0) {
            // WARNNING: free(pkt.ptr) if alloc new buffer
            break;
        }
        _onRead(m_pkt.track, m_pkt.ptr, m_pkt.bytes, m_pkt.pts, m_pkt.dts, m_pkt.flags);
    }

    return ret < 0 ? -1 : 0;
}

int CFmp4DeMuxer::process() {
    LOG_WARN("process into");
    while (State() == Running) {
        if (readerProcess() < 0) {
            break;
        }
        usleep(1000 * 1000);
    }
    LOG_WARN("process exit");
    return -1;
}
}  // namespace zc
