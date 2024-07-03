// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "ZcRtmpPush.hpp"
#include "ZcModCli.hpp"

namespace zc {
class CRtmpPushMan : public CModCli, public CRtmpPush {
 public:
    CRtmpPushMan();
    virtual ~CRtmpPushMan();

 public:
    bool Init(unsigned int type, unsigned int chn, const char *url);
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
