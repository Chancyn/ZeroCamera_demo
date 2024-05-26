// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <vector>
#define ZC_LIVE_TEST 1  // test read h264file

#if ZC_LIVE_TEST
#include <stdint.h>

#include <memory>
#include <string>

#include "media-source.h"

#include "time64.h"
#include "zc_frame.h"

#include "Singleton.hpp"
#include "Thread.hpp"
#include "ZcLiveTestWriter.hpp"
#include "ZcLiveTestWriterH264.hpp"
#include "ZcLiveTestWriterH265.hpp"

class CMediaTrack;
namespace zc {
class CLiveTestWriterFac {
 public:
    CLiveTestWriterFac() {}
    ~CLiveTestWriterFac() {}
    ILiveTestWriter *CreateLiveTestWriter(int code, const live_test_info_t &info) {
        ILiveTestWriter *pw = nullptr;
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
};

class CLiveTestWriterSys : public Singleton<CLiveTestWriterSys>  {
 public:
    CLiveTestWriterSys();
    virtual ~CLiveTestWriterSys();

 public:
    int Init();
    int UnInit();

 private:
    int m_init;
    std::vector<ILiveTestWriter *> m_vector;
    std::mutex m_mutex;
};
#define g_ZCLiveTestWriterInstance (CLiveTestWriterSys::GetInstance())

}  // namespace zc
#endif