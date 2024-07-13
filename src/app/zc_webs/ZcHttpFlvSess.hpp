
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_type.h"

#include "NonCopyable.hpp"

#define ZC_SUPPORT_HTTP_FLV 0       // support http-flv

#if ZC_SUPPORT_HTTP_FLV
#include "ZcFlvMuxer.hpp"
namespace zc {
class CHttpFlvSess : public NonCopyable {
 public:
    explicit CHttpFlvSess(const zc_flvmuxer_info_t &info);
    virtual ~CHttpFlvSess();
 private:
    static int OnFlvPacketCb(void *param, int type, const void *data, size_t bytes, ZC_U32 timestamp);
    static int _onFlvPacketCb(int type, const void *data, size_t bytes, ZC_U32 timestamp);

 private:
    CFlvMuxer flvmuxer;
    bool m_running;
    char m_addrs[32];   // https server addr
    void *m_handle;     // mongoose mgr
};

}  // namespace zc
#endif
