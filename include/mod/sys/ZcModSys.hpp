// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"

#include "ZcModBase.hpp"
#include "ZcMsg.hpp"
#include "sys/ZcMsgProcModSys.hpp"

namespace zc {
class CModSys : public CModBase {
 public:
    CModSys();
    virtual ~CModSys();

 public:
    virtual bool Init();
    virtual bool UnInit();

 private:
    bool _unInit();

 private:
    CMsgProcModSys *m_pMsgProc;
    int m_init;
};
}  // namespace zc
