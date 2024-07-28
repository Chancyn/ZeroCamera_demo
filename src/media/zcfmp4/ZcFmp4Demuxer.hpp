// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
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
#define ZC_FMP4_DEMUXER_DEBUG_SAVE (1)      // debug save

// readbuf maxsize
#define ZC_FMP4_RBUF_MAX (2 * 1024 * 1024)         // read MP4 buffer
#define ZC_FMP4_RBUF_ANNEXB_MAX (1 * 2024 * 1024)  // annexb read buffer

typedef struct {
    int flags;
    int64_t pts;
    int64_t dts;
    uint32_t track;

    void *ptr;
    size_t bytes;
} zc_mov_pkt_info_t;

typedef struct {
     char s_pts[64];
     char s_dts[64];
     int64_t v_pts;
     int64_t v_dts;
     int64_t a_pts;
     int64_t a_dts;
     int64_t x_pts;
     int64_t x_dts;
} zc_mov_ts_t;

class CFmp4DeMuxer : protected Thread {
    enum fmp4de_status_e {
        fmp4de_status_err = -1,  // error
        fmp4de_status_init = 0,  // init
        fmp4de_status_run,       // running
    };

 public:
    CFmp4DeMuxer();
    virtual ~CFmp4DeMuxer();
    bool Open(const char *name);
    bool Close();
    bool Start();
    bool Stop();

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
    static void OnRead(void *param, uint32_t track, const void *buffer, size_t bytes, int64_t pts, int64_t dts,
                       int flags);
    void _onRead(uint32_t track, const void *buffer, size_t bytes, int64_t pts, int64_t dts, int flags);
    static void *OnAlloc(void *param, uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags);
    void *_onAlloc(uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags);
    int readerProcess();
    virtual int process();

 private:
    int m_open;
    fmp4de_status_e m_status;
    std::string m_name;
    uint32_t m_bufferlen;
    uint32_t m_annexbbuflen;
    uint8_t *m_buffer;
    uint8_t *m_annexbbuf;
    CMovReadIo *m_movbuf;
    struct mov_reader_t *m_reader;
    uint64_t m_duration;
    zc_mov_pkt_info_t m_pkt;
    std::map<int, std::string> m_tracks;
    zc_mov_ts_t m_lastts;
    void *m_mpeg4video;
    void *m_mpeg4audio;
#if ZC_FMP4_DEMUXER_DEBUG_SAVE
    FILE *m_debug_vfp;
    FILE *m_debug_afp;
#endif
};
}  // namespace zc
