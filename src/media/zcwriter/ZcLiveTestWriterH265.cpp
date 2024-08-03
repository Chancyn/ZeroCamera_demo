// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <asm-generic/errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <list>
#include <mutex>

#include <memory>

//#include "rtp-payload.h"
//#include "rtp-profile.h"
//#include "rtp.h"
//#include "sys/path.h"
//#include "sys/system.h"

#include "zc_basic_fun.h"
#include "zc_h26x_sps_parse.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcLiveTestWriterH265.hpp"
#include "ZcType.hpp"
#include "zc_basic_stream.h"

#define ZC_DEBUG_DUMP 0  // debug dump
#if ZC_DEBUG_DUMP
#include "zc_basic_fun.h"
#endif

extern "C" uint32_t rtp_ssrc(void);

#if ZC_LIVE_TEST
namespace zc {
enum {
    NAL_BLA_W_LP = 16,
    NAL_BLA_W_RADL = 17,
    NAL_BLA_N_LP = 18,
    NAL_IDR_W_RADL = 19,
    NAL_IDR_N_LP = 20,
    NAL_CRA = 21,

    NAL_VPS = 32,
    NAL_SPS = 33,
    NAL_PPS = 34,
    NAL_SEI = 39
};
CLiveTestWriterH265::CLiveTestWriterH265(const live_test_info_t &info)
    : Thread(info.threadname), m_status(0), m_reader(nullptr), m_fifowriter(nullptr) {
    m_rtp_clock = 0;
    m_rtcp_clock = 0;
    m_timestamp = 0;
    m_seq = 0;
    memcpy(&m_info, &info, sizeof(m_info));
    memset(&m_naluinfo, 0, sizeof(m_naluinfo));
    if (info.fps == 0) {
        m_info.fps = 60;
    }
    m_clock_interal = 1000 / m_info.fps;
    Init();
    Start();
}

CLiveTestWriterH265::~CLiveTestWriterH265() {
    UnInit();
}

unsigned int CLiveTestWriterH265::_putingCb(void *stream) {
    test_raw_frame_t *frame = reinterpret_cast<test_raw_frame_t *>(stream);
    return m_fifowriter->PutAppending(frame->ptr, frame->len);
}

unsigned int CLiveTestWriterH265::putingCb(void *u, void *stream) {
    CLiveTestWriterH265 *self = reinterpret_cast<CLiveTestWriterH265 *>(u);
    return self->_putingCb(stream);
}

int CLiveTestWriterH265::Init() {
    LOG_TRACE("Init into");

    // m_fifowriter = new CShmFIFOW(ZC_STREAM_MAIN_VIDEO_SIZE, ZC_STREAM_VIDEO_SHM_PATH, 0);
    m_fifowriter = new CShmStreamW(m_info.size, m_info.fifopath, m_info.chn, putingCb, this);
    if (!m_fifowriter->ShmAlloc()) {
        LOG_ERROR("ShmAllocWrite error");
        ZC_ASSERT(0);
        return -1;
    }

    LOG_TRACE("Init OK");
    return 0;
}

int CLiveTestWriterH265::UnInit() {
    Stop();

    ZC_SAFE_DELETE(m_fifowriter);
    return 0;
}

int CLiveTestWriterH265::Play() {
    m_status = 1;
    return 0;
}

// >0 ok return sleep time, < 0 error,
int CLiveTestWriterH265::_putData2FIFO() {
#if ZC_LIVE_TEST
    int ret = 0;
    // uint32_t timestamp = 0;
    uint64_t clock = zc_system_clock();  // time64_now();
    if (0 == m_rtp_clock)
        m_rtp_clock = clock;

    if (m_rtp_clock + m_clock_interal <= clock) {
        size_t bytes;
        const uint8_t *ptr;
        int32_t idr = ZC_FRAME_P;

        if ((ret = m_reader->GetNextFrame(m_pos, ptr, bytes, &idr)) == 0) {
            // LOG_TRACE("Put bytes[%d]", bytes);
            // m_fifowriter->Put(ptr, bytes);
            zc_frame_t frame;
            memset(&frame, 0, sizeof(frame));
            frame.magic = ZC_FRAME_VIDEO_MAGIC;
            frame.size = bytes;
            frame.seq = m_seq++;
            frame.type = ZC_STREAM_VIDEO;
            frame.video.encode = m_info.encode;
            frame.video.width = m_naluinfo.width;
            frame.video.height = m_naluinfo.height;
            frame.keyflag = idr;
#if 0
            struct timespec _ts;
            clock_gettime(CLOCK_MONOTONIC, &_ts);
            frame.utc = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
            frame.pts = frame.utc;  // m_pos;
#else
            frame.utc = zc_system_time();
            frame.pts = m_rtp_clock;  // m_pos;
#endif

#if ZC_DUMP_BINSTREAM  // dump
            if (frame.keyflag) {
                zc_debug_dump_binstream(__FUNCTION__, ZC_FRAME_ENC_H265, ptr, bytes, 64);
                LOG_WARN("putseq:%d,IDR:%d,len:%d,pts:%u,utc:%u,wh[%u*%u]", frame.seq, frame.keyflag, frame.size,
                         frame.pts, frame.utc, frame.video.width, frame.video.height);
            }
#endif
            test_raw_frame_t raw;
            raw.ptr = ptr;
            raw.len = bytes;
            m_fifowriter->Put((const unsigned char *)&frame, sizeof(frame), &raw);
            m_rtp_clock += m_clock_interal;
            m_timestamp += m_clock_interal;

            return m_rtp_clock > clock ? (m_rtp_clock - clock) : 0;
        } else {
            LOG_ERROR("read file end,ret[%d]", ret);
        }
    } else {
        ret = m_rtp_clock + m_clock_interal - clock;
    }
#endif
    return ret;
}

int CLiveTestWriterH265::fillnaluInfo(zc_video_naluinfo_t &sdpinfo) {
    const std::list<std::pair<const uint8_t *, size_t>> &sps = m_reader->GetParameterSets();
    std::list<std::pair<const uint8_t *, size_t>>::const_iterator it;
    zc_video_naluinfo_t tmp = {0};

    unsigned int type = 0;
    tmp.nalunum = 0;
    for (it = sps.begin(); it != sps.end(); ++it) {
        unsigned int naltype = ((*(it->first)) & 0x7E) >> 1;
        size_t bytes = it->second;
        // LOG_TRACE("naluinfo %p, type:0x%d, size:%d", it->first, naltype, bytes);

        if (naltype == NAL_VPS) {
            type = ZC_NALU_TYPE_VPS;
        } else if (naltype == NAL_SPS) {
            type = ZC_NALU_TYPE_SPS;

            zc_h26x_sps_info_t spsinfo = {0};
            if (zc_h265_sps_parse(it->first, bytes, &spsinfo) == 0) {
                tmp.width = spsinfo.width;
                tmp.height = spsinfo.height;
                LOG_WARN("prase wh [wh:%hu:%hu]", spsinfo.width, spsinfo.height);
            }
        } else if (naltype == NAL_PPS) {
            type = ZC_NALU_TYPE_PPS;
        } else if (naltype == NAL_SEI) {
            type = ZC_NALU_TYPE_SEI;
        } else {
            LOG_ERROR("unsupport naluinfo %p, type:%d, size:%d", it->first, naltype, bytes);
            continue;
        }

        zc_nalu_t *nalu = &tmp.nalu[tmp.nalunum];
        if (bytes > 0 && bytes <= sizeof(nalu->data)) {
            memcpy(nalu->data, it->first, bytes);
            nalu->size = bytes;
            nalu->type = type;

#if ZC_DUMP_BINSTREAM  // dump
            LOG_WARN("fillSdpInfo num:%d, type:%d, size:%d", tmp.nalunum, type, nalu->size);
            zc_debug_dump_binstream("dump nalu", ZC_FRAME_ENC_H265, nalu->data,  nalu->size, nalu->size);
#endif
            tmp.nalunum++;
            if (tmp.nalunum >= ZC_FRAME_NALU_MAXNUM) {
                break;
            }
        }
    }

    memcpy(&sdpinfo, &tmp, sizeof(zc_video_naluinfo_t));

    return 0;
}

int CLiveTestWriterH265::process() {
    LOG_WARN("process into");
    int ret = 0;
    ZC_SAFE_DELETE(m_reader);
    m_reader = new H265FileReader(m_info.filepath);
    fillnaluInfo(m_naluinfo);
    while (State() == Running) {
        if (1 /*m_status == 1*/) {
            ret = _putData2FIFO();
            if (ret < 0) {
                LOG_WARN("process file EOF exit");
                ret = 0;
                goto _err;
            } else if (ret > 0) {
                ZC_MSLEEP(ret%20);
            } else {
                ZC_MSLEEP(1);
            }
        }
    }
_err:
    ZC_SAFE_DELETE(m_reader);
    LOG_WARN("process exit");
    return ret;
}

}  // namespace zc
#endif
