// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"
#include "zc_frame.h"

#include "ZcRtmpPull.hpp"
#include "ZcModCli.hpp"

namespace zc {
class CRtmpPullMan : public CModCli {
 public:
    CRtmpPullMan();
    virtual ~CRtmpPullMan();

 public:
    bool Init(unsigned int chn, const char *url);
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    static int getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info);
    int _getStreamInfoCb(unsigned int chn, zc_stream_info_t *info);
    static int setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info);
    int _setStreamInfoCb(unsigned int chn, zc_stream_info_t *info);
    bool _unInit();

 private:
    bool m_init;
    int m_running;
    uint32_t m_chn;
    zc_stream_info_t m_mediainfo;
    CIRtmpPull *m_pull;
};
}  // namespace zc

