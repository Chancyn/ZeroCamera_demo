// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include <vector>

#include "flv-demuxer.h"
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcShmStream.hpp"

namespace zc {

typedef struct {
    uint32_t seqno;
    uint32_t pts;
    uint32_t dts;
} zc_flv_dinfo_t;

typedef int (*flvOnFrame)(void* param, zc_frame_t *framehdr, const uint8_t *data);

typedef struct {
    flvOnFrame onframe;
    void *ctx;
} zc_flvdemuxer_info_t;

class CFlvDemuxer {
 public:
    explicit CFlvDemuxer(const zc_flvdemuxer_info_t &cb);
    virtual ~CFlvDemuxer();

 public:
    int Input(int type, const uint8_t *data, size_t bytes, uint32_t timestamp);

 private:
    static int onFlvCb(void *ptr, int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags);
    int _onFlvCb(int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags);

 private:
    flv_demuxer_t *m_flv;
    zc_flvdemuxer_info_t m_info;
    zc_flv_dinfo_t dframeinfo[ZC_MEDIA_TRACK_BUTT];
};

}  // namespace zc
