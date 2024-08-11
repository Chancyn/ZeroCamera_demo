// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"

#define ZC_SRT_PORT (10080)       // 10080

namespace zc {
#if 0
class CSrtSvr : public Thread {
 public:
    CSrtSvr();
    virtual ~CSrtSvr();

 public:
    bool Init(ZC_U16 port = ZC_SRT_PORT);
    bool UnInit();
    bool Start();
    bool Stop();
 private:
    bool _startsvr();
    bool _stopsvr();
    int _svrwork();
    virtual int process();

 private:
    bool m_init;
    int m_running;
    int m_status;
    ZC_U8 m_aiothreadnum;
    ZC_U16 m_port;
    char m_host[ZC_MAX_PATH];
    void *m_phandle;
    void *m_srtsvr;
    std::mutex m_mutex;
};
#endif
}  // namespace zc
