// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_h26x_sps_parse.h"

#include "ZcMediaReceiver.hpp"

namespace zc {

class CMediaReceiverH265 : public CMediaReceiver {
 public:
    explicit CMediaReceiverH265(int chn, unsigned int maxframelen = ZC_STREAM_MAXFRAME_SIZE);
    virtual ~CMediaReceiverH265();
    virtual bool Init(void *info = nullptr);
    virtual int RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags);
 private:
    static unsigned int putingCb(void *u, void *stream);
    unsigned int _putingCb(void *stream);

 private:
    zc_h26x_sps_info_t m_spsinfo;
    uint32_t m_lasttime;
    unsigned int m_pkgcnt;  // nalucnt, video frame pack by multiple nalu,
    zc_frame_t *m_frame;
};
}  // namespace zc
