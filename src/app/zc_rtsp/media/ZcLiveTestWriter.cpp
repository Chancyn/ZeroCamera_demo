// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <asm-generic/errno.h>
#include <stdio.h>
#include <sys/stat.h>

#include <mutex>

#include <memory>

#include "rtp-payload.h"
#include "rtp-profile.h"
#include "rtp.h"
#include "sys/path.h"
#include "sys/system.h"

#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcLiveTestWriter.hpp"
#include "ZcType.hpp"

extern "C" uint32_t rtp_ssrc(void);

#if ZC_LIVE_TEST
namespace zc {
typedef struct {
    const uint8_t *ptr;
    unsigned int len;
} test_raw_frame_t;

CLiveTestWriter::CLiveTestWriter() : Thread("LiveTestWriter"), m_status(0), m_alloc(0), m_reader(nullptr), m_fifowriter(nullptr) {
    m_rtp_clock = 0;
    m_rtcp_clock = 0;
    m_timestamp = 0;
    Init();
    Start();
}

CLiveTestWriter::~CLiveTestWriter() {
    UnInit();
}

unsigned int CLiveTestWriter::_putingCb(void *stream) {
    test_raw_frame_t *frame = reinterpret_cast<test_raw_frame_t *>(stream);
    return m_fifowriter->PutAppending(frame->ptr, frame->len);
}

unsigned int CLiveTestWriter::putingCb(void *u, void *stream) {
    CLiveTestWriter *self = reinterpret_cast<CLiveTestWriter *>(u);
    return self->_putingCb(stream);
}

int CLiveTestWriter::Init() {
    LOG_TRACE("Init into");
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        if (m_alloc != 0) {
            LOG_ERROR("already ShmAllocWrite");
            return -1;
        }
        // m_fifowriter = new CShmFIFOW(ZC_STREAM_MAIN_VIDEO_SIZE, ZC_STREAM_VIDEO_SHM_PATH, 0);
        m_fifowriter = new CShmStreamW(ZC_STREAM_MAIN_VIDEO_SIZE, ZC_STREAM_VIDEO_SHM_PATH, 0, putingCb, this);
        if (!m_fifowriter->ShmAlloc()) {
            LOG_ERROR("ShmAllocWrite error");
            ZC_ASSERT(0);
            m_alloc = -1;
            return -1;
        }
        m_alloc = 1;
    }
    LOG_TRACE("Init OK");
    return 0;
}

int CLiveTestWriter::UnInit() {
    Stop();

    ZC_SAFE_DELETE(m_fifowriter);
}

int CLiveTestWriter::Play() {
    m_status = 1;
    return 0;
}

int CLiveTestWriter::_putData2FIFO() {
#if ZC_LIVE_TEST
    int ret = 0;
    // uint32_t timestamp = 0;
    time64_t clock = time64_now();
    if (0 == m_rtp_clock)
        m_rtp_clock = clock;

    if (m_rtp_clock + 40 < clock) {
        size_t bytes;
        const uint8_t *ptr;
        bool idr = false;

        if ((ret = m_reader->GetNextFrame(m_pos, ptr, bytes, &idr)) == 0) {
            // LOG_TRACE("Put bytes[%d]", bytes);
            // m_fifowriter->Put(ptr, bytes);
            struct timespec _ts;
            clock_gettime(CLOCK_MONOTONIC, &_ts);
            zc_frame_t frame;
            frame.type = ZC_STREAM_VIDEO;
            frame.keyflag = idr;
            frame.size = bytes;
            frame.pts = m_pos;
            frame.utc = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;

            test_raw_frame_t raw;
            raw.ptr = ptr;
            raw.len = bytes;
            m_fifowriter->Put((const unsigned char *)&frame, sizeof(frame), &raw);
            m_rtp_clock += 40;
            m_timestamp += 40;
            return 1;
        } else {
            LOG_ERROR("read file end,ret[%d]", ret);
        }
    }
#endif
    return ret;
}

int CLiveTestWriter::process() {
    LOG_WARN("process into\n");
    int ret = 0;
    int64_t dts = 0;
    ZC_SAFE_DELETE(m_reader);
    m_reader = new H264FileReader(ZC_LIVE_TEST_FILE);
    while (State() == Running) {
        if (1 /*m_status == 1*/) {
            ret = _putData2FIFO();
            if (ret == -1) {
                LOG_WARN("process file EOF exit\n");
                ret = 0;
                goto _err;
            }
            usleep(10 * 1000);
        }
    }
_err:
    ZC_SAFE_DELETE(m_reader);
    LOG_WARN("process exit\n");
    return ret;
}

}  // namespace zc
#endif
