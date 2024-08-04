// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <asm-generic/errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <list>
#include <memory>
#include <mutex>

#include "zc_basic_fun.h"
#include "zc_h26x_sps_parse.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcLiveTestWriterMp4.hpp"

#include "ZcType.hpp"

extern "C" uint32_t rtp_ssrc(void);

#if ZC_LIVE_TEST

namespace zc {

typedef struct {
    CShmStreamW *fifow;
    unsigned int chn;
    unsigned int datalen;
    const uint8_t *dataptr;
} zc_write_frame_t;

static live_mp4_info_t g_livemp4infodef = {
    {
        {0, ZC_STREAM_MAIN_VIDEO_SIZE, ZC_STREAM_VIDEO_SHM_PATH},
        {0, ZC_STREAM_AUDIO_SIZE, ZC_STREAM_AUDIO_SHM_PATH},
        {0, ZC_STREAM_META_SIZE, ZC_STREAM_META_SHM_PATH},
    },
};

CLiveTestWriterMp4::CLiveTestWriterMp4(const live_test_info_t &info)
    : Thread(info.threadname), m_status(0), m_chn(info.chn), m_name(info.filepath), m_reader(nullptr) {
    m_last_vpts = 0;
    m_last_clock = 0;
    memcpy(&m_info, &g_livemp4infodef, sizeof(m_info));
    for (unsigned int i = 0; i < _SIZEOFTAB(m_info.stream); i++) {
        m_info.stream[i].chn = m_chn;
    }
    memset(&m_tracks, 0, sizeof(m_tracks));

    m_clock_interal = 1000 / 60;
    Init();
    Start();
}

CLiveTestWriterMp4::~CLiveTestWriterMp4() {
    UnInit();
}

unsigned int CLiveTestWriterMp4::_putingCb(void *stream) {
    zc_write_frame_t *writer = reinterpret_cast<zc_write_frame_t *>(stream);
    return writer->fifow->PutAppending(writer->dataptr, writer->datalen);
}

unsigned int CLiveTestWriterMp4::putingCb(void *u, void *stream) {
    CLiveTestWriterMp4 *self = reinterpret_cast<CLiveTestWriterMp4 *>(u);
    return self->_putingCb(stream);
}

int CLiveTestWriterMp4::Init() {
    LOG_TRACE("Init into");
    for (unsigned int i = 0; i < ZC_MEDIA_TRACK_BUTT; i++) {
        m_fifowriter[i] =
            new CShmStreamW(m_info.stream[i].size, m_info.stream[i].fifopath, m_info.stream[i].chn, putingCb, this);
        if (!m_fifowriter[i]->ShmAlloc()) {
            LOG_ERROR("ShmAllocWrite error");
            ZC_ASSERT(0);
            // return -1;
        }
    }

    LOG_TRACE("Init OK");
    Start();
    return 0;
}

int CLiveTestWriterMp4::UnInit() {
    Stop();
    for (unsigned int i = 0; i < ZC_MEDIA_TRACK_BUTT; i++) {
        ZC_SAFE_DELETE(m_fifowriter[i]);
    }
    return 0;
}

int CLiveTestWriterMp4::Play() {
    m_status = 1;
    return 0;
}

#if 1
void CLiveTestWriterMp4::_delayByPts(uint64_t pts) {
    if (m_last_vpts == 0) {
        m_last_vpts = pts;
    }
    uint64_t clock = zc_system_clock();
    if (m_last_clock == 0) {
        m_last_clock = clock;
    }
    if (pts > m_last_vpts) {
        int32_t delay = pts - m_last_vpts;
        if (clock < m_last_clock + delay) {
            delay = delay + m_last_clock - clock;
        } else {
            delay = 0;
        }
        if (abs(int32_t(delay - m_clock_interal)) >= 5) {
            LOG_WARN("delay:%d,pts:%llu->%llu,%d, clock:%llu->%llu,%d", delay, m_last_vpts, pts, pts - m_last_vpts,
                     m_last_clock, clock, clock - m_last_clock);
        }
        ZC_MSLEEP(delay);
    }
    clock = zc_system_clock();
    m_last_vpts = pts;
    m_last_clock = clock;

    return;
}

int CLiveTestWriterMp4::_putData2FIFO() {
    int ret = 0;
    zc_mov_frame_info_t movframe;
    // uint32_t timestamp = 0;

    if ((ret = m_reader->GetNextFrame(movframe)) >= 0) {
        CShmStreamW *fifowriter = m_fifowriter[movframe.stream % ZC_STREAM_BUTT];
        if (fifowriter && (movframe.stream == ZC_STREAM_VIDEO || movframe.stream == ZC_STREAM_AUDIO)) {
            // pts use video
            if ((m_tracks.tracks[ZC_STREAM_VIDEO].used && movframe.stream == ZC_STREAM_VIDEO) ||
                !m_tracks.tracks[ZC_STREAM_VIDEO].used) {
                _delayByPts(movframe.pts);
            }

            zc_frame_t frame;
            memset(&frame, 0, sizeof(frame));
            // frame.magic = ZC_FRAME_VIDEO_MAGIC;
            frame.size = movframe.bytes;
            frame.seq = movframe.seqno;
            frame.type = movframe.stream;
            frame.video.encode = movframe.encode;
            frame.utc = zc_system_time();
            frame.pts = movframe.pts;  // m_last_vpts;
            if (frame.type == ZC_STREAM_VIDEO) {
                frame.video.width = m_tracks.tracks[frame.type].vtinfo.width;
                frame.video.height = m_tracks.tracks[frame.type].vtinfo.height;
                frame.keyflag = movframe.keyflag;
            }

#if 1  // dump
            if (frame.keyflag) {
                LOG_TRACE("stream:%d, encode:%d, seq[%d] put IDR:%d len:%d, pts:%u,utc:%u, wh[%u*%u]", movframe.stream,
                          frame.video.encode, frame.seq, frame.keyflag, frame.size, frame.pts, frame.utc,
                          frame.video.width, frame.video.height);
            }
#endif
            zc_write_frame_t framedata;
            framedata.fifow = fifowriter;
            framedata.dataptr = movframe.ptr;
            framedata.datalen = movframe.bytes;
            fifowriter->Put((const unsigned char *)&frame, sizeof(frame), &framedata);

#if 0  // dump
            if (frame.type == ZC_STREAM_AUDIO) {
                zc_debug_dump_binstream("aac_hdr", ZC_STREAM_AUDIO, ( const uint8_t *)&frame, sizeof(frame), sizeof(frame));
                zc_debug_dump_binstream("adts_hdr", ZC_STREAM_AUDIO, framedata.dataptr, framedata.datalen, 64);
            }
#endif
            return 0;
        }
    }
    return ret;
}
#else
int CLiveTestWriterMp4::_putData2FIFO() {
    int ret = 0;
    zc_mov_frame_info_t movframe;
    // uint32_t timestamp = 0;
    uint64_t clock = zc_system_clock();  // time64_now();
    if (0 == m_last_vpts)
        m_last_vpts = clock;
    if (m_last_vpts + m_clock_interal <= clock) {
        if ((ret = m_reader->GetNextFrame(movframe)) >= 0) {
            CShmStreamW *fifowriter = m_fifowriter[movframe.stream % ZC_STREAM_BUTT];
            if (fifowriter && (movframe.stream == ZC_STREAM_VIDEO || movframe.stream == ZC_STREAM_AUDIO)) {
                zc_frame_t frame;
                memset(&frame, 0, sizeof(frame));
                // frame.magic = ZC_FRAME_VIDEO_MAGIC;
                frame.size = movframe.bytes;
                frame.seq = movframe.seqno;
                frame.type = movframe.stream;
                frame.video.encode = movframe.encode;
                if (movframe.stream == ZC_STREAM_VIDEO) {
                    frame.video.encode = movframe.encode;
                    frame.video.width = m_tracks.tracks[movframe.stream].vtinfo.width;
                    frame.video.height = m_tracks.tracks[movframe.stream].vtinfo.height;
                    frame.keyflag = movframe.keyflag;
                }
                frame.utc = zc_system_time();
                frame.pts = m_last_vpts;  // m_pos;

#if 1  // dump
                if (frame.keyflag) {
                    LOG_TRACE("stream:%d, encode:%d, seq[%d] put IDR:%d len:%d, pts:%u,utc:%u, wh[%u*%u]",
                              movframe.stream, frame.video.encode, frame.seq, frame.keyflag, frame.size, frame.pts,
                              frame.utc, frame.video.width, frame.video.height);
                }
#endif
                zc_write_frame_t framedata;
                framedata.fifow = fifowriter;
                framedata.dataptr = movframe.ptr;
                framedata.datalen = movframe.bytes;
                fifowriter->Put((const unsigned char *)&frame, sizeof(frame), &framedata);
                m_last_vpts += m_clock_interal;
                m_timestamp += m_clock_interal;

                return m_last_vpts > clock ? (m_last_vpts - clock) : 0;
            } else {
                // LOG_ERROR("read file end,ret[%d]", ret);
            }
        } else {
            ret = m_last_vpts + m_clock_interal - clock;
        }
        return ret;
    }
}
#endif

int CLiveTestWriterMp4::OnFrameCallback(void *param, const zc_mov_frame_info_t *frame) {
    return reinterpret_cast<CLiveTestWriterMp4 *>(param)->_onFrameCallback(frame);
}

int CLiveTestWriterMp4::_onFrameCallback(const zc_mov_frame_info_t *frame) {
    // todo;
    return 0;
}

int CLiveTestWriterMp4::process() {
    LOG_WARN("process into");
    int ret = 0;

    m_reader = new CFmp4DeMuxer();
    if (!m_reader->Open(m_name.c_str())) {
        LOG_ERROR("open error name:%s", m_name.c_str());
        goto _err;
    }
    LOG_WARN("open ok");
    m_reader->GetTrackInfo(m_tracks);
    while (State() == Running) {
        if (1 /*m_status == 1*/) {
            ret = _putData2FIFO();
            if (ret < 0) {
                LOG_WARN("process file EOF exit");
                ret = 0;
                goto _err;
            } else if (ret > 0) {
                ZC_MSLEEP(ret % m_clock_interal);
            }
        }
    }
_err:
    LOG_WARN("error ret:%d", ret);
    ZC_SAFE_DELETE(m_reader);
    LOG_WARN("process exit");
    return ret;
}

}  // namespace zc
#endif
