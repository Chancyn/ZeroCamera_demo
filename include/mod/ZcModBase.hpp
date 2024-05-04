// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_mod_base.h"
#include "zc_msg.h"

#include "MsgCommRepServer.hpp"
#include "MsgCommReqClient.hpp"
#include "ZcModComm.hpp"
#include "ZcMsgProcMod.hpp"
#include "zc_type.h"

namespace zc {
class CModBase : public CModComm {
 public:
    CModBase(ZC_U8 modid, const char *url);
    virtual ~CModBase();
    virtual bool Init() = 0;
    virtual bool UnInit() = 0;
 protected:
    bool registerMsgProcMod(CMsgProcMod *msgprocmod);
    bool unregisterMsgProcMod(CMsgProcMod *msgprocmod);
    bool init();
    bool unInit();

 private:
    ZC_S32 _svrRecvReqProc(char *req, int iqsize, char *rep, int *opsize);
    ZC_S32 _cliRecvRepProc(char *rep, int size);

 private:
    bool m_init;
    ZC_U8 m_modid;
    ZC_CHAR m_url[ZC_URL_SIZE];
    // msg handle
    CMsgProcMod *m_pmsgmodproc;
};
}  // namespace zc
