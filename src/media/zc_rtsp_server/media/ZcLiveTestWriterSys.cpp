// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <asm-generic/errno.h>
#include <stdio.h>
#include <sys/stat.h>

#include <memory>
#include <mutex>

#include "h264-file-reader.h"
#include "h265-file-reader.h"
#include "rtp-payload.h"
#include "rtp-profile.h"
#include "rtp.h"
#include "sys/path.h"
#include "sys/system.h"

#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcLiveTestWriterH264.hpp"
#include "ZcLiveTestWriterH265.hpp"
#include "ZcLiveTestWriterSys.hpp"
#include "ZcType.hpp"

extern "C" uint32_t rtp_ssrc(void);

#if ZC_LIVE_TEST
namespace zc {

const static live_test_info_t g_livetestinfo[ZC_STREAM_VIDEO_MAX_CHN] = {
    // {0, ZC_STREAM_MAIN_VIDEO_SIZE, ZC_FRAME_ENC_H265, ZC_STREAM_VIDEO_SHM_PATH, "test.h265"},
    {0, ZC_STREAM_MAIN_VIDEO_SIZE, ZC_FRAME_ENC_H265, ZC_STREAM_VIDEO_SHM_PATH, "test.h265", "TestWH265_0"},
    {1, ZC_STREAM_SUB_VIDEO_SIZE, ZC_FRAME_ENC_H264, ZC_STREAM_VIDEO_SHM_PATH, "test.h264", "TestWH264_1"},
};

CLiveTestWriterSys::CLiveTestWriterSys() : m_init(0) {
    Init();
}

CLiveTestWriterSys::~CLiveTestWriterSys() {
    UnInit();
}

int CLiveTestWriterSys::Init() {
    LOG_TRACE("Init into");

    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_init != 0) {
        LOG_ERROR("already ShmAllocWrite");
        return -1;
    }

    CLiveTestWriterFac fac;
    ILiveTestWriter *tmp = nullptr;
    for (unsigned int i = 0; i < ZC_STREAM_VIDEO_MAX_CHN; i++) {
        tmp = fac.CreateLiveTestWriter(g_livetestinfo[i].encode, g_livetestinfo[i]);
        ZC_ASSERT(tmp != nullptr);
        if (!tmp) {
            LOG_ERROR("init error");
            continue;
        }

        if (tmp->Init() != 0) {
            LOG_ERROR("init error");
            delete tmp;
            continue;
        }

        m_vector.push_back(tmp);
    }
    m_init = 1;
    LOG_TRACE("Init OK, size[%d]", m_vector.size());
    return 0;
}

int CLiveTestWriterSys::UnInit() {
    if (!m_init) {
        return -1;
    }

    auto iter = m_vector.begin();
    for (; iter != m_vector.end();) {
        ZC_SAFE_DELETE(*iter);
        iter = m_vector.erase(iter);
    }
    m_vector.clear();
    m_init = 0;
    return 0;
}

}  // namespace zc
#endif
