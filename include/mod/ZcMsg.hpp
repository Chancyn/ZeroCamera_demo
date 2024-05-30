// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <functional>

#include "zc_mod_base.h"
#include "zc_msg.h"
#include "zc_macros.h"

// define for msg register msg handle
#ifndef ZC_MSG_SHAREPTR
// CMsgBase raw ptr
#define REGISTER_MSG(mod, id, sid, reqfun, repfun) \
    do { \
        auto stdfunreq = std::bind(reqfun, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, \
                                   std::placeholders::_4); \
        auto stdfunrep = std::bind(repfun, this, std::placeholders::_1, std::placeholders::_2); \
        CMsgBase *pmsg = new CMsgBase(mod, id, sid, stdfunreq, stdfunrep); \
        ZC_ASSERT(pmsg != nullptr); \
        if (pmsg) { \
            if (!registerMsg(id, sid, pmsg)) { \
                LOG_ERROR("insert id[%d],sid[%d]", id, sid); \
                ZC_ASSERT(0); \
                delete pmsg; \
            } \
        } \
    } while (0);
#else
// TODO(zhoucc) msg handle CMsgBase shareptr
#endif

#if 0
typedef ZC_S32 (*MsgReqProcCb)(void *pcontext, zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
typedef ZC_S32 (*MsgRepProcCb)(void *pcontext, zc_msg_t *rep, int size);
#else
typedef std::function<ZC_S32(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize)> MsgReqProcCb;
typedef std::function<ZC_S32(zc_msg_t *rep, int size)> MsgRepProcCb;
#endif

namespace zc {
class CMsgBase {
 public:
    CMsgBase(ZC_U8 modid, ZC_U16 id, ZC_U16 sid, MsgReqProcCb reqcb, MsgRepProcCb m_repcb);
    virtual ~CMsgBase();
    virtual ZC_S32 MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    virtual ZC_S32 MsgRepProc(zc_msg_t *rep, int size);

 private:
    ZC_U8 m_modid;
    ZC_U16 m_id;
    ZC_U16 m_sid;
    MsgReqProcCb m_reqcb;
    MsgRepProcCb m_repcb;
};
}  // namespace zc
