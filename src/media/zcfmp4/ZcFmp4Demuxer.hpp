// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstdint>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <map>
#include <string>

#include "mov-reader.h"
#include "zc_frame.h"

#include "Thread.hpp"
#include "ZcMovBuf.hpp"

namespace zc {
#define ZC_FMP4DE_DEBUG_DUMP 0              // debug log
#define ZC_FMP4_DEMUXER_DEBUG_SAVE (1)      // debug save

// readbuf maxsize
#define ZC_FMP4_RBUF_MAX (2 * 1024 * 1024)         // read MP4 buffer

typedef struct {
    uint8_t flags;
    uint8_t encode;
    uint8_t stream;
    uint8_t object;
    int64_t pts;
    int64_t dts;
    uint32_t track;

    void *ptr;
    size_t bytes;
} zc_mov_pkt_info_t;

typedef struct {
    int keyflag;
    int64_t pts;
    int64_t dts;
    zc_frame_enc_e encode;
    zc_stream_e stream;
    uint8_t *ptr;
    size_t bytes;
} zc_mov_frame_info_t;

typedef struct {
     char s_pts[64];
     char s_dts[64];
} zc_mov_ts_t;

typedef struct {
    uint8_t used;
    uint8_t trackid;
    uint8_t object;
    zc_frame_enc_e encode;
    zc_stream_e stream;
    union {
        zc_video_trackinfo_t vtinfo;
        zc_audio_trackinfo_t atinfo;
    };
} zc_mov_track_t;

typedef struct {
    zc_mov_track_t tracks[ZC_MEDIA_TRACK_BUTT];
} zc_mov_trackinfo_t;

typedef int (*OnFmp4FrameCb)(void *param, const zc_mov_frame_info_t *frame);
typedef struct {
    OnFmp4FrameCb onfmp4framecb;
    void *Context;
} zc_fmp4demuxer_info_t;

class CFmp4DeMuxer : protected Thread {
    enum fmp4de_status_e {
        fmp4de_status_err = -1,  // error
        fmp4de_status_init = 0,  // init
        fmp4de_status_run,       // running
    };

 public:
    CFmp4DeMuxer();
    explicit CFmp4DeMuxer(const zc_fmp4demuxer_info_t &info);
    virtual ~CFmp4DeMuxer();
    bool Open(const char *name);
    bool Close();
    bool Start();
    bool Stop();
    int GetNextFrame(zc_mov_frame_info_t &frame);
    bool GetTrackInfo(zc_mov_trackinfo_t &tracks);
 private:
    static void OnMovVideoInfo(void *param, uint32_t track, uint8_t object, int width, int height, const void *extra,
                               size_t bytes);
    void _onMovVideoInfo(uint32_t track, uint8_t object, int width, int height, const void *extra, size_t bytes);
    static void OnMovAudioInfo(void *param, uint32_t track, uint8_t object, int channel_count, int bit_per_sample,
                               int sample_rate, const void *extra, size_t bytes);
    void _onMovAudioInfo(uint32_t track, uint8_t object, int channel_count, int bit_per_sample, int sample_rate,
                         const void *extra, size_t bytes);
    static void OnMovSubtitleInfo(void *param, uint32_t track, uint8_t object, const void *extra, size_t bytes);
    void _onMovSubtitleInfo(uint32_t track, uint8_t object, const void *extra, size_t bytes);

    static void *OnAlloc(void *param, uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags);
    void *_onAlloc(uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags);

    int _videopkt2frame(const zc_mov_pkt_info_t &pkt,  zc_mov_frame_info_t &frame);
    int _aduiopkt2frame(const zc_mov_pkt_info_t &pkt,  zc_mov_frame_info_t &frame);
    int _metapkt2frame(const zc_mov_pkt_info_t &pkt,  zc_mov_frame_info_t &frame);
    const zc_mov_track_t *_findTrackinfo(uint32_t track);
    int _getNextFrame(zc_mov_frame_info_t &frame);
    int _demuxerProcess();
    virtual int process();

 private:
    int m_open;
    fmp4de_status_e m_status;
    std::string m_name;
    zc_fmp4demuxer_info_t m_info;
    uint32_t m_bufferlen;
    uint32_t m_vframebuflen;
    uint32_t m_aframebuflen;
    uint8_t *m_buffer;
    uint8_t *m_vframebuf;
    uint8_t *m_aframebuf;
    zc_mov_frame_info_t m_vframe;
    zc_mov_frame_info_t m_aframe;
    CMovReadIo *m_movbuf;
    struct mov_reader_t *m_reader;
    uint64_t m_duration;
    zc_mov_pkt_info_t m_pkt;

    zc_mov_trackinfo_t m_tracksinfo;
    // std::map<int, zc_mov_track_t> m_tracks;
    zc_mov_ts_t m_lastts;
    void *m_mpeg4video;
    void *m_mpeg4audio;
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
    FILE *m_debug_vfp;
    FILE *m_debug_afp;
#endif
};
}  // namespace zc
