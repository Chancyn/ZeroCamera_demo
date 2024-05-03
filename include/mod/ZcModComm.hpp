// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_mod_base.h"
#include "zc_msg.h"

#include "MsgCommRepServer.hpp"
#include "MsgCommReqClient.hpp"

namespace zc {
class CModComm {
 public:
    CModComm();
    virtual ~CModComm();

    bool InitComm(const char *url, MsgCommReqSerHandleCb svrcb);
    bool UnInitComm();

 private:
    ZC_U8 m_url[ZC_URL_SIZE];
    bool m_init;
    CMsgCommRepServer *m_psvr;
};
}  // namespace zc
