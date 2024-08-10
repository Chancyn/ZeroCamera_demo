// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>

#include "zc_basic_fun.h"
#include "zc_log.h"

#include "ZcStreamTrace.hpp"
#include "ZcType.hpp"

namespace zc {
#if ZC_DEBUG_TRACE
const char *g_streamnametab[ZC_STREAM_BUTT] = {
    "video",
    "audio",
    "meta",
};

CStreamTrace::CStreamTrace(const zc_strace_cfg_t *cfg) {
#if ZC_DEBUG_TRACE
    if (cfg) {
        m_cfg = *cfg;
        m_cfg.interval = m_cfg.interval > 0 ? m_cfg.interval : ZC_DEBUG_TRACE_DEFINTREVAL;
    } else {
        m_cfg.enable[ZC_STREAM_VIDEO] = 1;
        m_cfg.enable[ZC_STREAM_AUDIO] = 1;
        m_cfg.interval = ZC_DEBUG_TRACE_DEFINTREVAL;
    }
    memset(&m_strace, 0, sizeof(m_strace));
#endif
}

CStreamTrace::~CStreamTrace() {}
void CStreamTrace::Init(const zc_strace_cfg_t &cfg) {
#if ZC_DEBUG_TRACE
    m_cfg = cfg;
    m_cfg.interval = m_cfg.interval > 0 ? m_cfg.interval : ZC_DEBUG_TRACE_DEFINTREVAL;
#endif
    return;
}

void CStreamTrace::ResetTrace(zc_stream_e type) {
#if ZC_DEBUG_TRACE
    if (ZC_STREAM_BUTT >= 0) {
        return;
    }

    memset(&m_strace[type], 0, sizeof(zc_strace_info_t));
#endif
    return;
}

int CStreamTrace::GetTraceInfo(zc_stream_e type, zc_strace_info_t *traceinfo) {
#if ZC_DEBUG_TRACE
    if (ZC_STREAM_BUTT >= 0 || traceinfo == nullptr) {
        return -1;
    }

    memcpy(traceinfo, &m_strace[type], sizeof(zc_strace_info_t));
#endif
    return 0;
}

void CStreamTrace::TraceStream(const zc_frame_t *frame) {
#if ZC_DEBUG_TRACE
    int type = frame->type % ZC_STREAM_BUTT;
    if (m_cfg.enable[type]) {
        zc_strace_info_t *trace = &m_strace[type];
        ZC_U64 now = zc_system_time();
        if (trace->startutc == 0) {
            trace->startutc = now;
            trace->lastpts = frame->pts;
            trace->lastseq = frame->seq;
            trace->size = 0;
            trace->framecnt = 0;
            trace->lostcnt = 0;
            trace->startframecnt = 0;
            return;
        }

        if (trace->lastseq + 1 != frame->seq) {
            trace->lostcnt++;
        }

        if (trace->fps > 1 && frame->pts != 0 && frame->pts != 0) {
            int diff = 1000 / trace->fps;
            if ((frame->pts - trace->lastpts) > 2 * diff) {
                LOG_WARN("%s frame delay fps:%.2f, pts:%llu->%llu,%d, diff:%d", g_streamnametab[type], trace->fps,
                         trace->lastpts, frame->pts, frame->pts - trace->lastpts, diff);
            } else if ((frame->pts - trace->lastpts) < 4) {
                LOG_WARN("%s early fps:%.2f, pts:%llu->%llu,%d, diff:%d", g_streamnametab[type], trace->fps,
                         trace->lastpts, frame->pts, frame->pts - trace->lastpts, diff);
            }
        }

        // check audio/video sync

        trace->lastpts = frame->pts;
        trace->lastseq = frame->seq;

        trace->framecnt++;
        trace->size += frame->size;
        if (now > (trace->startutc + 1000 * m_cfg.interval)) {
            ZC_U32 framediff = trace->framecnt - trace->startframecnt;
            ZC_U64 timediff = (now - trace->startutc) / 1000;
            trace->bitrate = trace->size / framediff * 1000 / 1024;
            trace->fps = (double) framediff / timediff;
            LOG_TRACE("%s:fps[%.2f],cnt:%u,bps:%ukbps,seq[%u->%u=%u],cos[%llu->%llu=%llu]ms", g_streamnametab[type],
                      (double)trace->fps, framediff, trace->bitrate, trace->startseq, frame->seq,
                      frame->seq - trace->startseq, trace->startutc, now, timediff);

            trace->size = 0;
            trace->startseq = frame->seq;
            trace->startutc = now;
            trace->startframecnt = trace->framecnt;
        }
    }
#endif
    return;
}

#endif
}  // namespace zc
