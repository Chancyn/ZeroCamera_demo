
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_type.h"

#include "NonCopyable.hpp"
#include "ZcFlvMuxer.hpp"
#include "ZcWebMSess.hpp"

#define ZC_SUPPORT_HTTP_FLV 1  // support http-flv
#define ZC_SUPPORT_WS_FLV 1    // support ws-flv,websocket-flv

namespace zc {
class CFlvSess : public IWebMSess, public NonCopyable {
 public:
    CFlvSess(zc_web_msess_type_e type, const zc_flvsess_info_t &info);
    virtual ~CFlvSess();
    bool Open();
    bool Close();
    bool StartSendProcess();
    void *GetConnSess() { return m_info.connsess; }

 private:
    static int OnFlvPacketCb(void *param, int type, const void *data, size_t bytes, ZC_U32 timestamp);
    virtual int _onFlvPacketCb(int type, const void *data, size_t bytes, ZC_U32 timestamp);

 private:
    zc_msess_status_e m_status;  // zc_flvsess_status_e
    CFlvMuxer m_flvmuxer;
    zc_flvsess_info_t m_info;
};
}  // namespace zc
