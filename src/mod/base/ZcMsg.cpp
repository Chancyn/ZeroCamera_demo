// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <string.h>

#include "zc_macros.h"

#include "ZcMsg.hpp"
#include "zc_log.h"

namespace zc {
CMsgBase::CMsgBase(ZC_U8 modid, ZC_U16 id, ZC_U16 sid, MsgReqProcCb reqcb, MsgRepProcCb repcb)
    : m_modid(modid), m_id(id), m_sid(sid), m_reqcb(reqcb), m_repcb(repcb) {}

CMsgBase::~CMsgBase() {}

ZC_S32 CMsgBase::MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("ReqProc into, modid[%u], id[%hu],m_sid[%hu]", m_modid, m_id, m_sid);
    ZC_ASSERT(req->id == m_id);
    ZC_ASSERT(req->sid == m_sid);
    if (m_reqcb)
        return m_reqcb(req, iqsize, rep, opsize);

    return -1;
}

ZC_S32 CMsgBase::MsgRepProc(zc_msg_t *rep, int size) {
    // LOG_TRACE("RepProc into, modid[%u], id[%hu],id[%hu]", m_modid, m_id, m_sid);
    ZC_ASSERT(rep->id == m_id);
    ZC_ASSERT(rep->sid == m_sid);
    if (m_repcb)
        return m_repcb(rep, size);

    return -1;
}


CMsgSubBase::CMsgSubBase(ZC_U8 modid, ZC_U16 id, ZC_U16 sid, MsgSubProcCb subcb)
    : m_modid(modid), m_id(id), m_sid(sid), m_subcb(subcb) {}

CMsgSubBase::~CMsgSubBase() {}

ZC_S32 CMsgSubBase::MsgSubProc(zc_msg_t *sub, int iqsize) {
    LOG_TRACE("ReqProc into, modid[%u], id[%hu],m_sid[%hu]", m_modid, m_id, m_sid);
    ZC_ASSERT(sub->id == m_id);
    ZC_ASSERT(sub->sid == m_sid);
    if (m_subcb)
        return m_subcb(sub, iqsize);

    return -1;
}

}  // namespace zc
