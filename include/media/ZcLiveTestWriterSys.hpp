// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#if ZC_LIVE_TEST
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "time64.h"
#include "zc_frame.h"

#include "Singleton.hpp"
#include "Thread.hpp"
#include "ZcLiveTestWriter.hpp"

class CMediaTrack;
namespace zc {
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
    int Init();
    int UnInit();

 private:
    int m_init;
    std::vector<ILiveTestWriter *> m_vector;
    std::mutex m_mutex;
};
#define g_ZCLiveTestWriterInstance (zc::CLiveTestWriterSys::GetInstance())

}  // namespace zc
#endif