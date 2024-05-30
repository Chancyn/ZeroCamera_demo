// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "ZcMsg.hpp"
#include "zc_mod_base.h"

namespace zc {
class CMsgProcMod {
 public:
    CMsgProcMod(ZC_U8 modid, ZC_U16 idmax);
    virtual ~CMsgProcMod();
    virtual bool Init() = 0;
    virtual bool UnInit() = 0;
    ZC_S32 MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 MsgRepProc(zc_msg_t *rep, int size);

 protected:
    bool registerMsg(ZC_U16 id, ZC_U16 sid, CMsgBase *pMsg);
    bool init();
    bool uninit();

 protected:
    ZC_U8 m_modid;
    ZC_U16 m_idmax;
    std::map<ZC_U32, CMsgBase *> m_msgmap;

 private:
    int m_init;
};
}  // namespace zc
