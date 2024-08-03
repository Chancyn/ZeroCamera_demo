// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#if ZC_LIVE_TEST
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

//#include "time64.h"
#include "zc_frame.h"

#include "Singleton.hpp"
#include "Thread.hpp"
#include "ZcLiveTestWriter.hpp"

class CMediaTrack;
namespace zc {
typedef int (*TestWGetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);
typedef int (*TestWSetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);

typedef struct {
    TestWGetInfoCb GetInfoCb;
    TestWSetInfoCb SetInfoCb;
    void *MgrContext;
} testwriter_callback_info_t;

class CLiveTestWriterFac {
 public:
    CLiveTestWriterFac() {}
    ~CLiveTestWriterFac() {}
    ILiveTestWriter *CreateLiveTestWriter(int code, const live_test_info_t &info);
};

class CLiveTestWriterSys : public Singleton<CLiveTestWriterSys> {
 public:
    CLiveTestWriterSys();
    virtual ~CLiveTestWriterSys();

 public:
    int Init(const testwriter_callback_info_t &cbinfo, unsigned int *pCodeTab = nullptr, unsigned int len = 0);
    int UnInit();

 private:
     int setStreamInfo();
     int init();

 private:
    int m_init;
    testwriter_callback_info_t m_cbinfo;
    live_test_info_t m_liveinfotab[ZC_STREAM_VIDEO_MAX_CHN];
    std::vector<ILiveTestWriter *> m_vector;
    std::mutex m_mutex;
};
#define g_ZCLiveTestWriterInstance (zc::CLiveTestWriterSys::GetInstance())

}  // namespace zc
#endif