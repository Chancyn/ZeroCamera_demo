// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#define ZC_LIVE_TEST 1  // test read h264file

#if ZC_LIVE_TEST
#include <stdint.h>

#include <memory>
#include <string>

#include "media-source.h"

#include "h264-file-reader.h"
#include "time64.h"
#include "zc_frame.h"

#include "Singleton.hpp"
#include "Thread.hpp"
#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"

#define ZC_LIVE_TEST_FILE "test.h264"

class CMediaTrack;
namespace zc {
class CLiveTestWriter : public Thread, public Singleton<CLiveTestWriter> {
 public:
    CLiveTestWriter();
    virtual ~CLiveTestWriter();

 public:
    virtual int Play();
    int Init();
    int UnInit();

 private:
    int _putData2FIFO();
    virtual int process();
    static unsigned int putingCb(void *u, void *stream);
    unsigned int _putingCb(void *stream);

 private:
    int m_status;
    int m_alloc;
    H264FileReader *m_reader;
    // CShmFIFOW *m_fifowriter;
    CShmStreamW *m_fifowriter;
    uint32_t m_timestamp;
    uint64_t m_rtp_clock;
    uint64_t m_rtcp_clock;
    int64_t m_pos;
    std::mutex m_mutex;
};
#define g_ZCLiveTestWriterInstance (CLiveTestWriter::GetInstance())

}  // namespace zc
#endif