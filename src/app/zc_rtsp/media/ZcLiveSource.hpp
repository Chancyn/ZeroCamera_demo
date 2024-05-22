// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <stdint.h>

#include <string>
#include <memory>

#include "media-source.h"

#include "ZcMediaTrack.hpp"

#define ZC_LIVE_TEST 1  // test read h264file

#if ZC_LIVE_TEST
#include "h264-file-reader.h"
#include "time64.h"
#include "zc_media_fifo_def.h"

#include "Thread.hpp"
#include "ZcShmFIFO.hpp"

#define ZC_LIVE_TEST_FILE "test.h264"
#endif

class CMediaTrack;
namespace zc {
class CLiveSource : public IMediaSource, public Thread {
 public:
    CLiveSource();
    virtual ~CLiveSource();

 public:
    virtual int Play();
    virtual int Pause();
    virtual int Seek(int64_t pos);
    virtual int SetSpeed(double speed);
    virtual int GetDuration(int64_t &duration) const;
    virtual int GetSDPMedia(std::string &sdp) const;
    virtual int GetRTPInfo(const char *uri, char *rtpinfo, size_t bytes) const;
    virtual int SetTransport(const char *track, std::shared_ptr<IRTPTransport> transport);
    int Init();
    int UnInit();
 private:
    int SendBye();
    virtual int process();
    int _sendProcess();
 private:
    std::string m_sdp;
    int m_status;
    CMediaTrack *m_tracks[MEDIA_TRACK_BUTT];
    int m_count;
#if ZC_LIVE_TEST
    H264FileReader m_reader;
    CShmFIFOW *m_fifowriter;
    uint32_t m_timestamp;
    uint64_t m_rtp_clock;
    uint64_t m_rtcp_clock;
    int64_t m_pos;
#endif
};
}  // namespace zc
