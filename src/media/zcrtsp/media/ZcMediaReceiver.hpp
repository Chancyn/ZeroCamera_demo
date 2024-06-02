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

#define ZC_DEBUG_MEDIATRACK 1

namespace zc {


class CMediaReceiver {
 public:
    explicit CMediaReceiver(zc_media_track_e track, int code, int shmtype, int chn, unsigned int m_maxframelen);
    virtual ~CMediaReceiver();
    virtual bool Init(void *info = nullptr) = 0;
    virtual void UnInit();
    // rtp receiver onframe in, put stream to
    virtual int RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags);
 private:

 protected:
    bool m_create;  // create ok
    int m_track;    // trackid
    int m_code;     // codectype
    const zc_shmstream_e m_shmtype;  // push/pull
    const int m_chn;
    unsigned int m_maxframelen;
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
