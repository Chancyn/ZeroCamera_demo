// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#if ZC_LIVE_TEST
#include <stdint.h>

#include <memory>
#include <string>

#include "h265-file-reader.h"
#include "time64.h"
#include "zc_frame.h"

#include "Singleton.hpp"
#include "Thread.hpp"
#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"
#include "ZcLiveTestWriter.hpp"

class CMediaTrack;
namespace zc {
class CLiveTestWriterH265 : public Thread, public ILiveTestWriter{
 public:
    explicit CLiveTestWriterH265(const live_test_info_t &info);
    virtual ~CLiveTestWriterH265();

 public:
    virtual int Play();
    virtual int Init();
    virtual int UnInit();

 private:
    int _putData2FIFO();
    virtual int process();
    static unsigned int putingCb(void *u, void *stream);
    unsigned int _putingCb(void *stream);
    int fillnaluInfo(zc_video_naluinfo_t &sdpinfo);

 private:
    int m_status;
    live_test_info_t m_info;
    H265FileReader *m_reader;
    // CShmFIFOW *m_fifowriter;
    CShmStreamW *m_fifowriter;
    uint32_t m_seq;
    uint32_t m_timestamp;
    uint32_t m_clock_interal;
    uint64_t m_rtp_clock;
    uint64_t m_rtcp_clock;
    int64_t m_pos;
    zc_video_naluinfo_t m_naluinfo;
};

}  // namespace zc
#endif