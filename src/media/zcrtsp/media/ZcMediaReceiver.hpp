// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <memory>
#include <string>

#include "media-source.h"
#include "zc_frame.h"
#include "zc_media_track.h"

#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"
#include "ZcType.hpp"
#include "NonCopyable.hpp"

#define ZC_DEBUG_MEDIATRACK 1

namespace zc {


class CMediaReceiver : public NonCopyable{
 public:
    explicit CMediaReceiver(const zc_meida_track_t &info);
    virtual ~CMediaReceiver();
    virtual bool Init(void *info = nullptr) = 0;
    virtual void UnInit();
    // rtp receiver onframe in, put stream to
    virtual int RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags);
    virtual int SetRtpInfo_Rtptime(uint16_t seq, uint32_t timestamp, uint64_t npt);
 private:

 protected:
    bool m_create;  // create ok
    zc_meida_track_t m_info;
    char *m_framebuf;      // sizeof(zc_frame_t)+ZC_STREAM_MAXFRAME_SIZE
    CShmStreamW *m_fifowriter;
 private:
    uint32_t m_lasttime;

    // unsigned char m_framebuf[sizeof(zc_frame_t)+ZC_STREAM_MAXFRAME_SIZE];     // framebuf TODO(zhoucc): different size new
#if ZC_DEBUG_MEDIATRACK
    uint64_t m_debug_cnt_lasttime;
    uint32_t m_debug_framecnt_last;
    uint32_t m_debug_framecnt;
#endif

};
}  // namespace zc
