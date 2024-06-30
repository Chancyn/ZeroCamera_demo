// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"

#include "ZcModBase.hpp"
#include "ZcMsg.hpp"
#include "rtsp/ZcMsgProcModRtsp.hpp"

namespace zc {
class CModRtsp : public CModBase {
 public:
    CModRtsp();
    virtual ~CModRtsp();

 public:
    virtual bool Init(void *cbinfo);
    virtual bool UnInit();

 private:
    bool _unInit();

 private:
    CMsgProcModRtsp *m_pMsgProc;
    int m_init;
};
}  // namespace zc
