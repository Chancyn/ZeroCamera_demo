// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#if ZC_LIVE_TEST
#include <stdint.h>

#include <memory>
#include <string>

#include "zc_frame.h"

#include "Singleton.hpp"
#include "Thread.hpp"
#include "ZcFmp4Demuxer.hpp"
#include "ZcLiveTestWriter.hpp"
#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"

class CMediaTrack;
namespace zc {

typedef struct {
    unsigned int chn;
    unsigned int size;
    char fifopath[128];
} live_stream_info_t;

typedef struct {
    live_stream_info_t stream[ZC_MEDIA_TRACK_BUTT];
} live_mp4_info_t;

class CLiveTestWriterMp4 : public Thread, public ILiveTestWriter {
 public:
    explicit CLiveTestWriterMp4(const live_test_info_t &info);
    virtual ~CLiveTestWriterMp4();

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
    static int OnFrameCallback(void *param, const zc_mov_frame_info_t *frame);
    int _onFrameCallback(const zc_mov_frame_info_t *frame);
    void _delayByPts(uint64_t pts);
 private:
    int m_status;
    int m_chn;
    std::string m_name;
    live_mp4_info_t m_info;
    CFmp4DeMuxer *m_reader;
    CShmStreamW *m_fifowriter[ZC_MEDIA_TRACK_BUTT];
    uint32_t m_clock_interal;
    uint64_t m_last_vpts;
    uint64_t m_last_clock;
    zc_mov_trackinfo_t m_tracks;
};

}  // namespace zc
#endif