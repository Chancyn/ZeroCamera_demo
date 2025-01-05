// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <asm-generic/errno.h>
#include <stdio.h>

#include "sys/path.h"
#include "sys/system.h"
#include <memory>

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcMediaReceiver.hpp"

namespace zc {
CMediaReceiver::CMediaReceiver(const zc_meida_track_t &info) : m_create(false), m_fifowriter(nullptr) {
    memcpy(&m_info, &info, sizeof(zc_meida_track_t));
    m_framebuf = new char[m_info.framemaxlen + sizeof(zc_frame_t)]();
    m_lasttime = 0;
#if ZC_DEBUG_MEDIATRACK
    m_debug_cnt_lasttime = 0;
    m_debug_framecnt_last = 0;
    m_debug_framecnt = 0;
#endif
}

CMediaReceiver::~CMediaReceiver() {
    UnInit();
    ZC_SAFE_DELETEA(m_framebuf);
}

void CMediaReceiver::UnInit() {
    if (m_create) {
        ZC_SAFE_DELETE(m_fifowriter);
        m_create = false;
    }

    return;
}

int CMediaReceiver::RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags) {
    // default do nothing, not recv data
    return 0;
}

int CMediaReceiver::SetRtpInfo_Rtptime(uint16_t seq, uint32_t timestamp, uint64_t npt) {
    // default do nothing, not recv data
    return 0;
}
}  // namespace zc
