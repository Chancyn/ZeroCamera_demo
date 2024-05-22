// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#define ZC_LIVE_TEST 1  // test read h264file

#if ZC_LIVE_TEST
#include <stdint.h>

#include <string>
#include <memory>

#include "media-source.h"

#include "h264-file-reader.h"
#include "time64.h"
#include "zc_media_fifo_def.h"

#include "Thread.hpp"
#include "Singleton.hpp"
#include "ZcShmFIFO.hpp"

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

 private:
    H264FileReader *m_reader;
    int m_status;
    int m_alloc;
    CShmFIFOW *m_fifowriter;
    uint32_t m_timestamp;
    uint64_t m_rtp_clock;
    uint64_t m_rtcp_clock;
    int64_t m_pos;
    std::mutex m_mutex;
};
#define g_ZCLiveTestWriterInstance (CLiveTestWriter::GetInstance())
#endif
}  // namespace zc
