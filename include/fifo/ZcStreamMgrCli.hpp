// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <mutex>
#include <list>

#include "zc_type.h"
#include "zc_stream_mgr.h"

#include "Singleton.hpp"
#include "Thread.hpp"

namespace zc {
// TODO(zhoucc): MgrCli; -> Mgr save handle

class CStreamMgrCli : public Thread, public Singleton<CStreamMgrCli> {
 public:
    CStreamMgrCli();
    virtual ~CStreamMgrCli();

 public:
    bool Init(zc_streamcli_t *info);
    bool UnInit();
    bool Start();
    bool Stop();

    // handle msg ModMsg
    int GetShmStreamInfo(zc_shmstream_info_t *info, zc_shmstream_type_e type, unsigned int nchn);
 private:
    int connectSvr();
    bool _unInit();
    virtual int process();

 private:
    bool m_init;
    int m_running;
    zc_streamcli_t m_info;
    std::mutex m_mutex;
};

#define g_ZCStreamMgrCliInstance (zc::CStreamMgrCli::GetInstance())
}  // namespace zc
