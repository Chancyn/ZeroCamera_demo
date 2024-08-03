// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <asm-generic/errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <memory>
#include <mutex>

#include "h264-file-reader.h"
#include "h265-file-reader.h"

#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcLiveTestWriterH264.hpp"
#include "ZcLiveTestWriterH265.hpp"
#include "ZcLiveTestWriterMp4.hpp"
#include "ZcLiveTestWriterSys.hpp"
#include "ZcType.hpp"

extern "C" uint32_t rtp_ssrc(void);

// test framerate
#define ZC_TEST_FPS 60

#if ZC_LIVE_TEST
namespace zc {
static const char *g_encsuffix[ZC_FRAME_ENC_BUTT] = {
    "h264",
    "h265",
    "aac",
    "metabin",
};

static const char *g_filesuffix[ZC_FRAME_ENC_BUTT] = {
    "",
    ".mp4",
};
const static live_test_info_t g_livetestinfo[ZC_STREAM_VIDEO_MAX_CHN] = {
    // {0, ZC_STREAM_MAIN_VIDEO_SIZE, ZC_FRAME_ENC_H265, ZC_TEST_FPS, ZC_STREAM_VIDEO_SHM_PATH, "test"},
    // test0.h265
    {0, ZC_STREAM_MAIN_VIDEO_SIZE, ZC_TEST_FILE_MP4, ZC_FRAME_ENC_H265, ZC_TEST_FPS, ZC_STREAM_VIDEO_SHM_PATH, "test0",
     "TestW_0"},
    // test0.h264
    {1, ZC_STREAM_SUB_VIDEO_SIZE, ZC_TEST_FILE_MP4, ZC_FRAME_ENC_H264, ZC_TEST_FPS, ZC_STREAM_VIDEO_SHM_PATH, "test1",
     "TestW_1"},
};

ILiveTestWriter *CLiveTestWriterFac::CreateLiveTestWriter(int file, int code, const live_test_info_t &info) {
    ILiveTestWriter *pw = nullptr;
    // file type
    switch (file) {
    case ZC_TEST_FILE_MP4:
        pw = new CLiveTestWriterMp4(info);
        return pw;
    default:
        break;
    }

    switch (code) {
    case ZC_FRAME_ENC_H264:
        pw = new CLiveTestWriterH264(info);
        break;
    case ZC_FRAME_ENC_H265:
        pw = new CLiveTestWriterH265(info);
        break;
    default:
        LOG_ERROR("error, modid[%d]", pw);
        break;
    }
    return pw;
}

CLiveTestWriterSys::CLiveTestWriterSys() : m_init(0) {
    // Init();
}

CLiveTestWriterSys::~CLiveTestWriterSys() {
    UnInit();
}

int CLiveTestWriterSys::init() {
    LOG_TRACE("init into");
    CLiveTestWriterFac fac;
    ILiveTestWriter *tmp = nullptr;
    for (unsigned int i = 0; i < ZC_STREAM_VIDEO_MAX_CHN; i++) {
        tmp = fac.CreateLiveTestWriter(m_liveinfotab[i].file, m_liveinfotab[i].encode, m_liveinfotab[i]);
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

    LOG_TRACE("Init OK, size[%d]", m_vector.size());
    return 0;
}

static int transEncode2MediaCode(unsigned int encode) {
    int mediacode = -1;
    if (encode == ZC_FRAME_ENC_H264) {
        mediacode = ZC_MEDIA_CODE_H264;
    } else if (encode == ZC_FRAME_ENC_H265) {
        mediacode = ZC_MEDIA_CODE_H265;
    } else if (encode == ZC_FRAME_ENC_AAC) {
        mediacode = ZC_MEDIA_CODE_AAC;
    } else if (encode == ZC_FRAME_ENC_META_BIN) {
        mediacode = ZC_MEDIA_CODE_METADATA;
    }

    return mediacode;
}

int CLiveTestWriterSys::setStreamInfo() {
    LOG_TRACE("setStreamInfo into");
    zc_stream_info_t stinfo[ZC_STREAM_VIDEO_MAX_CHN] = {0};
    for (unsigned int i = 0; i < ZC_STREAM_VIDEO_MAX_CHN; i++) {
        if (!m_cbinfo.GetInfoCb || m_cbinfo.GetInfoCb(m_cbinfo.MgrContext, i, &stinfo[i]) < 0) {
            LOG_ERROR("testwriter m_cbinfo.GetInfoCb ");
            return 0;
        }
    }

    // update streaminfo
    for (unsigned int i = 0; i < ZC_STREAM_VIDEO_MAX_CHN; i++) {
        // reset encode video
        if (stinfo[i].tracks[0].encode != m_liveinfotab[i].encode) {
            LOG_WARN("update video chn:%u, encode:%u->%u", i, stinfo[i].tracks[0].encode, m_liveinfotab[i].encode);
            stinfo[i].tracks[0].encode = m_liveinfotab[i].encode;
            stinfo[i].tracks[0].mediacode = transEncode2MediaCode(stinfo[i].tracks[0].encode);
        }
        //
        if (!m_cbinfo.SetInfoCb || m_cbinfo.SetInfoCb(m_cbinfo.MgrContext, i, &stinfo[i]) < 0) {
            LOG_ERROR("testwriter m_cbinfo.SetInfoCb ");
            // return 0;
        }
    }

    LOG_TRACE("setStreamInfo end");
    return 0;
}

int CLiveTestWriterSys::Init(const testwriter_callback_info_t &cbinfo, unsigned int *pCodeTab, unsigned int len) {
    LOG_TRACE("Init into");
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_init != 0) {
        LOG_ERROR("already ShmAllocWrite");
        return -1;
    }
    memcpy(&m_cbinfo, &cbinfo, sizeof(m_cbinfo));

    // init encode type
    memcpy(&m_liveinfotab, &g_livetestinfo, sizeof(m_liveinfotab));
    for (unsigned int i = 0; i < ZC_STREAM_VIDEO_MAX_CHN; i++) {
        int code = g_livetestinfo[i].encode;
        if (pCodeTab && i < len) {
            code = (pCodeTab[i]) % ZC_FRAME_ENC_BUTT;
        }
        m_liveinfotab[i].encode = code;
        snprintf(m_liveinfotab[i].filepath, sizeof(m_liveinfotab[i].filepath) - 1, "%s.%s%s",
                 g_livetestinfo[i].filepath, g_encsuffix[code], g_filesuffix[m_liveinfotab[i].file]);
        LOG_TRACE("init testwriter,i:%u,code:%u,%s", i, code, m_liveinfotab[i].filepath);
    }

    if (init() < 0) {
        LOG_ERROR("Init error");
        return -1;
    }

    setStreamInfo();
    m_init = 1;

    LOG_TRACE("Init OK");
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
