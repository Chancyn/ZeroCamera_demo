// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
// #include <sys/syslog.h>
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
#include "zc_basic_fun.h"
#include "zc_h26x_sps_parse.h"
// #include "sys/system.h"
#include "zc_frame.h"
#include "zc_log.h"

#include "ZcFmp4Demuxer.hpp"
#include "ZcType.hpp"
#include "zc_macros.h"
#include "zc_type.h"

namespace zc {

#define ADTS_HEADER_LEN 7

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
      m_vframebuflen(ZC_STREAM_MAXFRAME_SIZE), m_aframebuflen(ZC_STREAM_MAXFRAME_SIZE_A), m_buffer(nullptr),
      m_vframebuf(nullptr), m_aframebuf(nullptr), m_movbuf(nullptr), m_reader(nullptr), m_duration(0),
      m_mpeg4video(nullptr), m_mpeg4audio(nullptr) {
    memset(&m_pkt, 0, sizeof(m_pkt));
    memset(&m_lastts, 0, sizeof(m_lastts));

    memset(&m_tracksinfo, 0, sizeof(m_tracksinfo));
    memset(&m_info, 0, sizeof(zc_fmp4demuxer_info_t));
}

CFmp4DeMuxer::CFmp4DeMuxer(const zc_fmp4demuxer_info_t &info)
    : Thread("mp4demuxer"), m_open(0), m_status(fmp4de_status_init), m_bufferlen(ZC_FMP4_RBUF_MAX),
      m_vframebuflen(ZC_STREAM_MAXFRAME_SIZE), m_aframebuflen(ZC_STREAM_MAXFRAME_SIZE_A), m_buffer(nullptr),
      m_vframebuf(nullptr), m_aframebuf(nullptr), m_movbuf(nullptr), m_reader(nullptr), m_duration(0),
      m_mpeg4video(nullptr), m_mpeg4audio(nullptr) {
    memset(&m_pkt, 0, sizeof(m_pkt));
    memset(&m_lastts, 0, sizeof(m_lastts));
    memcpy(&m_info, &info, sizeof(zc_fmp4demuxer_info_t));

    memset(&m_tracksinfo, 0, sizeof(m_tracksinfo));
}

CFmp4DeMuxer::~CFmp4DeMuxer() {
    Stop();
    Close();
}

void CFmp4DeMuxer::OnMovVideoInfo(void *param, uint32_t track, uint8_t object, int width, int height, const void *extra,
                                  size_t bytes) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onMovVideoInfo(track, object, width, height, extra, bytes);
}

void CFmp4DeMuxer::_onMovVideoInfo(uint32_t track, uint8_t object, int width, int height, const void *extra,
                                   size_t bytes) {
    LOG_WARN("onaudio track:%u,obj:%u,bytes:%zu,width:%d,height:%d,", track, object, bytes, width, height);
    zc_mov_track_t &trackinfo = m_tracksinfo.tracks[ZC_MEDIA_TRACK_VIDEO];
    trackinfo.used = 1;
    trackinfo.trackid = track;
    trackinfo.object = object;
    trackinfo.stream = ZC_STREAM_VIDEO;
    trackinfo.vtinfo.width = width;
    trackinfo.vtinfo.height = height;

    if (MOV_OBJECT_H264 == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_vfp = fopen("v.h264", "wb");
#endif

        trackinfo.encode = ZC_FRAME_ENC_H264;
        struct mpeg4_avc_t *avc = new struct mpeg4_avc_t();
        m_mpeg4video = avc;
        ZC_ASSERT(m_mpeg4video);
        mpeg4_avc_decoder_configuration_record_load((const uint8_t *)extra, bytes, avc);
    } else if (MOV_OBJECT_HEVC == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_vfp = fopen("v.h265", "wb");
#endif
        trackinfo.encode = ZC_FRAME_ENC_H265;
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
        trackinfo.encode = ZC_FRAME_ENC_AV1;
        struct aom_av1_t *av1 = new struct aom_av1_t();
        m_mpeg4video = av1;
        ZC_ASSERT(m_mpeg4video);
        aom_av1_codec_configuration_record_load((const uint8_t *)extra, bytes, av1);
    } else if (MOV_OBJECT_VP9 == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_vfp = fopen("v.vp9", "wb");
#endif
        trackinfo.encode = ZC_FRAME_ENC_VP9;
        struct webm_vpx_t *vpx = new struct webm_vpx_t();
        m_mpeg4video = vpx;
        ZC_ASSERT(m_mpeg4video);
        webm_vpx_codec_configuration_record_load((const uint8_t *)extra, bytes, vpx);
    }
#endif
    else {
        LOG_WARN("unsupport video encode:%u", object);
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
    LOG_WARN("onaudio track:%u,obj:%u,bytes:%zu,channel:%d,sample_bits:%d,sample_rate:%d", track, object, bytes,
             channel_count, bit_per_sample, sample_rate);
    zc_mov_track_t &trackinfo = m_tracksinfo.tracks[ZC_MEDIA_TRACK_AUDIO];
    trackinfo.used = 1;
    trackinfo.trackid = track;
    trackinfo.object = object;
    trackinfo.seqno = 0;
    trackinfo.stream = ZC_STREAM_AUDIO;
    trackinfo.atinfo.channels = channel_count;
    trackinfo.atinfo.sample_bits = bit_per_sample / 8;
    trackinfo.atinfo.sample_rate = sample_rate;
    if (MOV_OBJECT_AAC == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.aac", "wb");
#endif
        trackinfo.encode = ZC_FRAME_ENC_AAC;
        struct mpeg4_aac_t *aac = new struct mpeg4_aac_t();
        m_mpeg4audio = aac;
        ZC_ASSERT(m_mpeg4audio);
        ZC_ASSERT(bytes == (size_t)mpeg4_aac_audio_specific_config_load((const uint8_t *)extra, bytes, aac));
        ZC_ASSERT(channel_count == aac->channels);
        ZC_ASSERT(MOV_OBJECT_AAC == object);
        LOG_WARN("AAC track:%u,obj:%u,channel:%d,profile:%d", track, object, channel_count, aac->profile);
        zc_debug_dump_binstream("aac_extra", ZC_FRAME_ENC_AAC, (const uint8_t *)extra, bytes, bytes);
        // aac->profile = MPEG4_AAC_LC;
        aac->channel_configuration = channel_count;
        aac->sampling_frequency_index = mpeg4_aac_audio_frequency_from(sample_rate);
    }
#if 0
    else if (MOV_OBJECT_OPUS == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.opus", "wb");
#endif
        trackinfo = "OPUS";
        struct opus_head_t *opus = new struct opus_head_t();
        m_mpeg4audio = opus;
        ZC_ASSERT(m_mpeg4audio);
        ZC_ASSERT(bytes == opus_head_load((const uint8_t *)extra, bytes, opus));
        ZC_ASSERT(opus->input_sample_rate == 48000);
    } else if (MOV_OBJECT_MP3 == object || MOV_OBJECT_MP1A == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.mp3", "wb");
#endif
        trackinfo = "MP3";
    } else if (MOV_OBJECT_G711a == object || MOV_OBJECT_G711u == object) {
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        m_debug_afp = fopen("a.raw", "wb");
#endif
        trackinfo.encode = ZC_FRAME_ENC_AAC;
    }
#endif
    else {
        LOG_WARN("unsupport audio encode:%u", object);
        // s_aac_track = track;
        // s_aac.channel_configuration = channel_count;
        // s_aac.sampling_frequency_index = mpeg4_aac_audio_frequency_from(sample_rate);
        trackinfo.object = object;
    }

    return;
}

void CFmp4DeMuxer::OnMovSubtitleInfo(void *param, uint32_t track, uint8_t object, const void *extra, size_t bytes) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onMovSubtitleInfo(track, object, extra, bytes);
}

void CFmp4DeMuxer::_onMovSubtitleInfo(uint32_t track, uint8_t object, const void *extra, size_t bytes) {
    LOG_WARN("onsubtitle track:%u,obj:%u,bytes:%d,extra:%s", track, object, bytes, extra);
    // trackinfo = "SUBTITLE";
    zc_mov_track_t &trackinfo = m_tracksinfo.tracks[ZC_MEDIA_TRACK_META];
    trackinfo.used = 1;
    trackinfo.object = object;
    trackinfo.stream = ZC_STREAM_META;
    trackinfo.encode = ZC_FRAME_ENC_META_BIN;

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

    m_vframebuf = new uint8_t[m_vframebuflen];
    if (!m_vframebuf) {
        LOG_ERROR("reader new annexb buf error");
        goto _err;
    }

    m_aframebuf = new uint8_t[m_aframebuflen];
    if (!m_vframebuf) {
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
        LOG_ERROR("reader getinfo error");
        goto _err;
    }
    LOG_TRACE("WARNING :%s, m_duration:%llu", m_name.c_str(), m_duration);
    m_name = name;
    m_open = 1;
    LOG_TRACE("Open ok:%s", m_name.c_str());
    return true;
_err:
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
    if (m_debug_afp) {
        fclose(m_debug_afp);
        m_debug_afp = nullptr;
    }

    if (m_debug_vfp) {
        fclose(m_debug_vfp);
        m_debug_vfp = nullptr;
    }
#endif
    mov_reader_destroy(m_reader);
    m_reader = nullptr;
    ZC_SAFE_DELETEA(m_aframebuf);
    ZC_SAFE_DELETEA(m_vframebuf);
    ZC_SAFE_DELETEA(m_buffer);
    ZC_SAFE_DELETE(m_movbuf);
    LOG_TRACE("Open error:%s", name);
    return false;
}

bool CFmp4DeMuxer::Close() {
    if (!m_open) {
        return false;
    }
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
    if (m_debug_afp) {
        fclose(m_debug_afp);
        m_debug_afp = nullptr;
    }

    if (m_debug_vfp) {
        fclose(m_debug_vfp);
        m_debug_vfp = nullptr;
    }
#endif
    mov_reader_destroy(m_reader);
    m_reader = nullptr;
    ZC_SAFE_DELETEA(m_aframebuf);
    ZC_SAFE_DELETEA(m_vframebuf);
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

int CFmp4DeMuxer::_videopkt2frame(const zc_mov_pkt_info_t &pkt, zc_mov_frame_info_t &frame) {
    int n = 0;
    if (pkt.encode == ZC_FRAME_ENC_H264) {
        assert(h264_is_new_access_unit((const uint8_t *)pkt.ptr + 4, pkt.bytes - 4));
        n = h264_mp4toannexb((const struct mpeg4_avc_t *)m_mpeg4video, pkt.ptr, pkt.bytes, m_vframebuf, m_vframebuflen);
    } else if (pkt.encode == ZC_FRAME_ENC_H265) {
        // uint8_t nalu_type = (((const uint8_t *)pkt.ptr)[4] >> 1) & 0x3F;
        // uint8_t irap = (16 <= nalu_type) && (nalu_type <= 23);
        assert(h265_is_new_access_unit((const uint8_t *)pkt.ptr + 4, pkt.bytes - 4));
        n = h265_mp4toannexb((const struct mpeg4_hevc_t *)m_mpeg4video, pkt.ptr, pkt.bytes, m_vframebuf,
                             m_vframebuflen);
    }
#if 0
    else if (pkt.encode == ZC_FRAME_ENC_AV1) {
    // TODO(zhoucc): test
    // n = aom_av1_codec_configuration_record_save((const struct aom_av1_t* )m_mpeg4video, m_vframebuf,
    // m_vframebuflen);
    } else if (pkt.encode == ZC_FRAME_ENC_VP9) {
        // TODO(zhoucc): test
        // n = webm_vpx_codec_configuration_record_save((const struct webm_vpx_t* )m_mpeg4video, m_vframebuf,
        // m_vframebuflen);
    }
#endif
    else {
        LOG_ERROR("error video[%d], diff: %03d/%03d, bytes: %u", pkt.object, (pkt.pts - m_vframe.pts),
                  (pkt.dts - m_vframe.dts), (unsigned int)pkt.bytes);
        // assert(0);
    }

    if (n > 0) {
#if ZC_FMP4DE_DEBUG_DUMP
        if (pkt.flags) {
            LOG_TRACE("video enc:%d, pts:%s,dts:%s,diff:%03d/%03d,bytes: %u,flags:%d,n:%d", pkt.encode,
                      ftimestamp(pkt.pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)),
                      ftimestamp(pkt.dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pkt.pts - m_vframe.pts),
                      (pkt.dts - m_vframe.dts), (unsigned int)pkt.bytes, pkt.flags, n);
        }
#endif
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        fwrite(m_vframebuf, 1, n, m_debug_vfp);
#endif
        m_vframe.bytes = n;
        m_vframe.seqno = pkt.seqno;
        m_vframe.pts = pkt.pts;
        m_vframe.dts = pkt.dts;
        m_vframe.encode = (zc_frame_enc_e)pkt.encode;
        m_vframe.stream = ZC_STREAM_VIDEO;
        m_vframe.keyflag = pkt.flags;
        m_vframe.ptr = m_vframebuf;
        memcpy(&frame, &m_vframe, sizeof(zc_mov_frame_info_t));
    }

    return n;
}

int CFmp4DeMuxer::_aduiopkt2frame(const zc_mov_pkt_info_t &pkt, zc_mov_frame_info_t &frame) {
    int n = 0;
    if (pkt.encode == ZC_FRAME_ENC_AAC) {
        uint8_t adts[32];
        n = mpeg4_aac_adts_save((const struct mpeg4_aac_t *)m_mpeg4audio, pkt.bytes, adts, sizeof(adts));
        memcpy(m_aframebuf, adts, n);
        memcpy(m_aframebuf + n, pkt.ptr, pkt.bytes);
        n += pkt.bytes;
    }
#if 0
    else if (pkt.encode == ZC_FRAME_ENC_OPUS) {
        memcpy(m_aframebuf, pkt.ptr, pkt.bytes);
        n = pkt.bytes;
    } else if (pkt.encode == ZC_FRAME_ENC_MP3) {
        memcpy(m_aframebuf, pkt.ptr, pkt.bytes);
        n = pkt.bytes;
    } else if (pkt.encode == ZC_FRAME_ENC_G711) {
        memcpy(m_aframebuf, pkt.ptr, pkt.bytes);
        n = pkt.bytes;
    }
#endif
    else {
        LOG_ERROR("error audio:%d, diff: %03d/%03d, bytes: %u", pkt.object, (pkt.pts - m_vframe.pts),
                  (pkt.dts - m_vframe.dts), (unsigned int)pkt.bytes);
        // assert(0);
    }

    if (n > 0) {
#if 0  // ZC_FMP4DE_DEBUG_DUMP
LOG_WARN("audio enc:%d pts: %s, dts: %s, diff: %03d/%03d, bytes: %u, n:%d",
         ftimestamp(pkt.pts, m_lastts.s_pts, sizeof(m_lastts.s_pts)), pkt.encode,
         ftimestamp(pkt.dts, m_lastts.s_dts, sizeof(m_lastts.s_dts)), (pkt.pts - m_aframe.pts),
         (pkt.dts - m_aframe.dts), (unsigned int)pkt.bytes, n);
#endif
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
        fwrite(m_aframebuf, 1, n, m_debug_afp);
#endif

        m_aframe.bytes = n;
        m_aframe.seqno = pkt.seqno;
        m_aframe.pts = pkt.pts;
        m_aframe.dts = pkt.dts;
        m_aframe.encode = (zc_frame_enc_e)pkt.encode;
        m_aframe.stream = ZC_STREAM_AUDIO;
        m_aframe.keyflag = pkt.flags;
        m_aframe.ptr = m_aframebuf;
        memcpy(&frame, &m_aframe, sizeof(zc_mov_frame_info_t));
    }

    return n;
}

int CFmp4DeMuxer::_metapkt2frame(const zc_mov_pkt_info_t &pkt, zc_mov_frame_info_t &frame) {
    // zhoucc:TODO
    int n = 0;
    LOG_WARN("subtilte frame: %zu", pkt.bytes);
    // n = pkt.bytes;

    if (n > 0) {
        m_aframe.bytes = n;
        m_aframe.seqno = pkt.seqno;
        m_aframe.pts = pkt.pts;
        m_aframe.dts = pkt.dts;
        m_aframe.encode = (zc_frame_enc_e)pkt.encode;
        m_aframe.stream = ZC_STREAM_META;
        m_aframe.keyflag = pkt.flags;
        m_aframe.ptr = m_aframebuf;
        memcpy(&frame, &m_aframe, sizeof(zc_mov_frame_info_t));
    }

    return n;
}

void *CFmp4DeMuxer::OnAlloc(void *param, uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags) {
    return reinterpret_cast<CFmp4DeMuxer *>(param)->_onAlloc(track, bytes, pts, dts, flags);
}

zc_mov_track_t *CFmp4DeMuxer::_findTrackinfo(uint32_t track) {
    for (unsigned int i = 0; i < _SIZEOFTAB(m_tracksinfo.tracks); i++) {
        if (m_tracksinfo.tracks[i].used && m_tracksinfo.tracks[i].trackid == track) {
            return &m_tracksinfo.tracks[i];
        }
    }

    LOG_ERROR("not find track:%u", track);
    ZC_ASSERT(0);
    return nullptr;
}

void *CFmp4DeMuxer::_onAlloc(uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags) {
    // emulate allocation
    if (m_pkt.bytes < bytes)
        return nullptr;
    zc_mov_track_t *ptrack = _findTrackinfo(track);
    m_pkt.encode = ptrack->encode;
    m_pkt.stream = ptrack->stream;
    m_pkt.object = ptrack->object;
    m_pkt.seqno = (ptrack->seqno)++;
    m_pkt.flags = flags;
    m_pkt.pts = pts;
    m_pkt.dts = dts;
    m_pkt.track = track;
    m_pkt.bytes = bytes;
    return m_pkt.ptr;
}

int CFmp4DeMuxer::_getNextFrame(zc_mov_frame_info_t &frame) {
    int ret = 0;
    m_pkt.ptr = m_buffer;
    m_pkt.bytes = m_bufferlen;
    if ((ret = mov_reader_read2(m_reader, OnAlloc, this)) <= 0) {
        LOG_ERROR("read error:%d", ret);
        return -1;
    }

    if (m_pkt.stream == ZC_STREAM_VIDEO) {
        _videopkt2frame(m_pkt, frame);
    } else if (m_pkt.stream == ZC_STREAM_AUDIO) {
        _aduiopkt2frame(m_pkt, frame);
    } else if (m_pkt.stream == ZC_STREAM_AUDIO) {
        _metapkt2frame(m_pkt, frame);
    }

    return 1;
}

int CFmp4DeMuxer::_demuxerProcess() {
    int ret = 0;
    zc_mov_frame_info_t frame;
    while (State() == Running) {
        ret = _getNextFrame(frame);
        if (ret < 0) {
            break;
        }

        if (m_info.onfmp4framecb) {
            m_info.onfmp4framecb(m_info.Context, &frame);
        }

        usleep(1000 * m_duration / 2);
    }

    return ret < 0 ? -1 : 0;
}

int CFmp4DeMuxer::process() {
    LOG_WARN("process into");
    while (State() == Running) {
        if (_demuxerProcess() < 0) {
            break;
        }
        usleep(1000 * 1000);
    }
    LOG_WARN("process exit");
    return -1;
}

int CFmp4DeMuxer::GetNextFrame(zc_mov_frame_info_t &frame) {
    if (!m_open) {
        return -1;
    }

    // LOG_WARN("get frame");
    return _getNextFrame(frame);
}

bool CFmp4DeMuxer::GetTrackInfo(zc_mov_trackinfo_t &tracks) {
    if (!m_open) {
        return false;
    }
    memcpy(&tracks, &m_tracksinfo, sizeof(zc_mov_trackinfo_t));
    return true;
}

}  // namespace zc
