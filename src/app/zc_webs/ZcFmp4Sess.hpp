
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_type.h"

#include "NonCopyable.hpp"
#include "ZcFmp4Muxer.hpp"
#include "ZcWebMSess.hpp"

#define ZC_SUPPORT_HTTP_FMP4 1  // support http-fmp4
#define ZC_SUPPORT_WS_FMP4 1    // support ws-fmp4,websocket-fmp4

namespace zc {
class CFmp4Sess : public IWebMSess, public NonCopyable {
 public:
    CFmp4Sess(zc_web_msess_type_e type, const zc_fmp4sess_info_t &info);
    virtual ~CFmp4Sess();
    bool Open();
    bool Close();
    bool StartSendProcess();
    void *GetConnSess() { return m_info.connsess; }

 private:
    static int OnFmp4PacketCb(void *param, int type, const void *data, size_t bytes, ZC_U32 timestamp);
    virtual int _onFmp4PacketCb(int type, const void *data, size_t bytes, ZC_U32 timestamp);

 private:
    zc_msess_status_e m_status;  // zc_fmp4sess_status_e
    CFmp4Muxer m_fmp4muxer;
    zc_fmp4sess_info_t m_info;
};
}  // namespace zc
