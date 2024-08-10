// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <pthread.h>
#include <unistd.h>

#include "zc_frame.h"

namespace zc {
#ifdef ZC_DEBUG
#define ZC_DEBUG_TRACE 1
#endif
#define ZC_DEBUG_TRACE_DEFINTREVAL (10)  // 默认统计间隔

typedef struct _zc_strace_cfg {
    ZC_U8 enable[ZC_STREAM_BUTT];
    ZC_U32 interval;
} zc_strace_cfg_t;

typedef struct _zc_strace_info {
    ZC_U64 lastpts;        // 上一帧pts
    ZC_U64 startutc;       // 阶段统计开始utc
    ZC_U32 startseq;       // 阶段统计开始seq
    ZC_U64 size;           // 数据流量大小
    ZC_U32 lastseq;        // 阶段统计开始seq
    ZC_U32 startframecnt;  // 阶段统计总帧数
    ZC_U32 framecnt;       // 帧数
    ZC_U32 lostcnt;        // 丢帧数量
    ZC_U32 bitrate;        // 平均码流 Kbps
    double fps;            // 帧率
} zc_strace_info_t;

class CStreamTrace {
 public:
    explicit CStreamTrace(const zc_strace_cfg_t *cfg = nullptr);
    virtual ~CStreamTrace();
    void Init(const zc_strace_cfg_t &cfg);
    void ResetTrace(zc_stream_e type);
    void TraceStream(const zc_frame_t *frame);
    int GetTraceInfo(zc_stream_e type, zc_strace_info_t *traceinfo);

 private:
    zc_strace_cfg_t m_cfg;
    zc_strace_info_t m_strace[ZC_STREAM_BUTT];
};

}  // namespace zc
