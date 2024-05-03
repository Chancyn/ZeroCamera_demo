// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_mod_base.h"

#include "MsgCommRepServer.hpp"
#include "MsgCommReqClient.hpp"

namespace zc {
class CModBase {
 public:
    CModBase(ZC_U8 modid);
    virtual ~CModBase();

 private:
    ZC_U8 m_modid;

    CMsgCommRepServer m_svr;
    CMsgCommReqClient m_cli;
};
}
