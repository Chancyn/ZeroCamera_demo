// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "ZcModCli.hpp"
#include "ZcWebServer.hpp"

#define ZC_WEBS_BITMASK_ALL (0xFFFF)
// http
// #define ZC_WEBS_BITMASK_DEF (0x1 << zc_webs_type_http)
// http+https
// #define ZC_WEBS_BITMASK_DEF ((0x1 << zc_webs_type_https) | 0x1)
// http+ws
// #define ZC_WEBS_BITMASK_DEF ((0x1 << zc_webs_type_ws) | 0x1)
// https+ws
// #define ZC_WEBS_BITMASK_DEF ((0x1 << zc_webs_type_wss) | (0x1 << zc_webs_type_https))

#define ZC_WEBS_BITMASK_DEF  ZC_WEBS_BITMASK_ALL

namespace zc {
class CWebServerMan : public CModCli {
 public:
    CWebServerMan();
    virtual ~CWebServerMan();

 public:
    bool Init(void *info = nullptr);
    bool UnInit();
    bool Start();
    bool Stop();

 private:
   //  static int getStreamInfoCb(void *ptr, unsigned int type, unsigned int chn, zc_stream_info_t *info);
   //  int _getStreamInfoCb(unsigned int type, unsigned int chn, zc_stream_info_t *info);

 private:
    bool m_init;
    int m_running;
    ZC_U8 m_supbitmask;      // support bitmask zc_webs_type_e bit0:http; bit1:https;bit2:ws;bit3:wss
    CWebServer *m_webstab[zc_webs_type_butt];
};
}  // namespace zc
