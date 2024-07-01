// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "ZcRtspPushClient.hpp"
#include "ZcModCli.hpp"

namespace zc {
class CRtspPushCliMan : public CModCli, public CRtspPushClient {
 public:
    CRtspPushCliMan();
    virtual ~CRtspPushCliMan();

 public:
    bool Init(unsigned int type, unsigned int chn, const char *url, int transport = ZC_RTSP_TRANSPORT_RTP_UDP);
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
