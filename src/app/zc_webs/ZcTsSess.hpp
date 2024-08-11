
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_type.h"

#include "NonCopyable.hpp"
#include "ZcTsMuxer.hpp"
#include "ZcWebMSess.hpp"

#define ZC_SUPPORT_HTTP_TS 1  // support http-ts
#define ZC_SUPPORT_WS_TS 1    // support ws-ts,websocket-ts

namespace zc {
class CTsSess : public IWebMSess, public NonCopyable {
 public:
    CTsSess(zc_web_msess_type_e type, const zc_tssess_info_t &info);
    virtual ~CTsSess();
    bool Open();
    bool Close();
    bool StartSendProcess();
    void *GetConnSess() { return m_info.connsess; }

 private:
    static int OnTsPacketCb(void *param, const void *data, size_t bytes);
    virtual int _onTsPacketCb(const void *data, size_t bytes);

 private:
    zc_msess_status_e m_status;  // zc_tssess_status_e
    CTsMuxer m_tsmuxer;
    zc_tssess_info_t m_info;
};
}  // namespace zc
