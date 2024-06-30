// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "ZcRtspServer.hpp"
#include "rtsp/ZcModRtsp.hpp"

namespace zc {
class CRtspManager : public CModRtsp, public CRtspServer {
 public:
    CRtspManager();
    virtual ~CRtspManager();

 public:
    bool Init(rtsp_callback_info_t *cbinfo);
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    static int getStreamInfoCb(void *ptr, unsigned int type, unsigned int chn, zc_media_info_t *info);
    int _getStreamInfoCb(unsigned int type, unsigned int chn, zc_media_info_t *info);
    bool _unInit();
    int _sendSMgrGetInfo(unsigned int type, unsigned int chn, zc_media_info_t *info);
 private:
    bool m_init;
    int m_running;

    rtsp_callback_info_t m_cbinfo;
    zc_media_info_t *m_pmediainfo;
};
}  // namespace zc
