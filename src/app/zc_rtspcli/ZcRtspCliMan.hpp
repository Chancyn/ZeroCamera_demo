// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "ZcModCli.hpp"
#include "ZcRtspClient.hpp"

namespace zc {
class CRtspCliMan : public CModCli, public CRtspClient {
 public:
    CRtspCliMan();
    virtual ~CRtspCliMan();

 public:
    bool Init(unsigned int chn, const char *url, int transport = ZC_RTSP_TRANSPORT_RTP_UDP);
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    bool _unInit();
    static int getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info);
    int _getStreamInfoCb(unsigned int chn, zc_stream_info_t *info);
    static int setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info);
    int _setStreamInfoCb(unsigned int chn, zc_stream_info_t *info);

 private:
    bool m_init;
    int m_running;
    zc_stream_info_t m_mediainfo;
};
}  // namespace zc
