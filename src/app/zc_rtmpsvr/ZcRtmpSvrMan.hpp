// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "ZcRtmpSvr.hpp"
#include "ZcModCli.hpp"

namespace zc {
class CRtmpSvrMan : public CModCli, public CRtmpSvr {
 public:
    CRtmpSvrMan();
    virtual ~CRtmpSvrMan();

 public:
    bool Init(ZC_U16 port);
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    static int getStreamInfoCb(void *ptr, unsigned int type, unsigned int chn, zc_stream_info_t *info);
    int _getStreamInfoCb(unsigned int type, unsigned int chn, zc_stream_info_t *info);
    bool _unInit();

 private:
    bool m_init;
    int m_running;
};
}  // namespace zc
