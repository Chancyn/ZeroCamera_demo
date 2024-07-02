// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"

#include "ZcModBase.hpp"
#include "ZcMsg.hpp"


namespace zc {
class CModSubBase : public CModBase  {
 public:
    explicit CModSubBase(ZC_U8 modid);
    virtual ~CModSubBase();
    bool init();
    bool unInit();
 private:
    ZC_S32 reqSvrRecvReqProc(char *req, int iqsize, char *rep, int *opsize);
    int _sendRegisterMsg(int cmd);
    int _sendKeepaliveMsg();

    virtual int modprocess();
 private:
    bool m_init;
    int m_status;
};
}  // namespace zc
