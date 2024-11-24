// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include <vector>

#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcShmStream.hpp"
#include "ZcStreamTrace.hpp"
// #include "ZcFile.hpp"

namespace zc {
typedef int (*tsOnFrame)(void *param, zc_frame_t *framehdr, const uint8_t *data);
typedef struct {
    uint32_t seqno;
    uint32_t pts;
    uint32_t dts;
} zc_ts_dinfo_t;

typedef struct {
    tsOnFrame onframe;
    void *ctx;
} zc_tsdemuxer_info_t;

class CTsDemuxer {
 public:
    explicit CTsDemuxer(const zc_tsdemuxer_info_t &cb);
    virtual ~CTsDemuxer();

 public:
    int Input(const uint8_t *data, size_t bytes);

 private:
    static int onTsCb(void *ptr, int program, int stream, int codecid, int flags, int64_t pts, int64_t dts,
                      const void *data, size_t bytes);
    int _onTsCb(int program, int stream, int codecid, int flags, int64_t pts, int64_t dts, const void *data,
                size_t bytes);

 private:
    zc_tsdemuxer_info_t m_info;
    void *m_ts;
    zc_ts_dinfo_t dframeinfo[ZC_MEDIA_TRACK_BUTT];
    //  CStreamTrace m_trace;
   //  CFile m_demuxerouttrace;
   //  CFile m_demuxerintrace;
};

}  // namespace zc
