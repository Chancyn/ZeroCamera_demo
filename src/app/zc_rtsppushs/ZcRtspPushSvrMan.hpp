// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "ZcRtspPushServer.hpp"
#include "zc_type.h"

#include "ZcModCli.hpp"

namespace zc {
class CRtspPushSvrMan : public CModCli, public CRtspPushServer {
 public:
    CRtspPushSvrMan();
    virtual ~CRtspPushSvrMan();

 public:
    bool Init();
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    bool _unInit();
    int _sendSMgrGetInfo(unsigned int type, unsigned int chn, zc_media_info_t *info);
    int _sendSMgrSetInfo(unsigned int type, unsigned int chn, const zc_media_info_t *info);
    static int getStreamInfoCb(void *ptr, unsigned int chn, zc_media_info_t *info);
    int _getStreamInfoCb(unsigned int chn, zc_media_info_t *info);
    static int setStreamInfoCb(void *ptr, unsigned int chn, zc_media_info_t *info);
    int _setStreamInfoCb(unsigned int chn, zc_media_info_t *info);

 private:
    bool m_init;
    int m_running;
    zc_media_info_t m_mediainfo;
};
}  // namespace zc
