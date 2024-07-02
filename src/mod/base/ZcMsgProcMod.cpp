// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <map>
#include <string.h>
#include <time.h>
#include <utility>

#include "zc_log.h"
#include "zc_type.h"

#include "ZcType.hpp"

#include "ZcMsg.hpp"
#include "ZcMsgProcMod.hpp"

namespace zc {
CMsgProcMod::CMsgProcMod(ZC_U8 modid, ZC_U16 idmax) : m_modid(modid), m_idmax(idmax), m_init(false) {
    LOG_TRACE("Constructor into, modid[%u] msgidmax[%u]", modid, idmax);
}

CMsgProcMod::~CMsgProcMod() {
    LOG_TRACE("Destructor into");
    uninit();
}

ZC_S32 CMsgProcMod::MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    // LOG_TRACE("MsgReqProc into, pid:%d,modid:%u, id[%hu],id[%hu]", req->pid, req->modid, req->id, req->sid);
    ZC_U32 key = (req->id << 16) | req->sid;
    auto it = m_msgmap.find(key);
    if (it != m_msgmap.end()) {
        return it->second->MsgReqProc(req, iqsize, rep, opsize);
    }

    return ZC_MSG_ERR_CMDID_E;
}

ZC_S32 CMsgProcMod::MsgRepProc(zc_msg_t *rep, int size) {
    if (!m_init) {
        return -1;
    }

    // LOG_TRACE("RepProc into, modid[%u], id[%hu],id[%hu]", m_modid, rep->id, rep->sid);
    ZC_U32 key = (rep->id << 16) | rep->sid;
    auto it = m_msgmap.find(key);
    if (it != m_msgmap.end()) {
        return it->second->MsgRepProc(rep, size);
    }

    return 0;
}

bool CMsgProcMod::registerMsg(ZC_U16 id, ZC_U16 sid, CMsgBase *pMsg) {
    if (m_init || pMsg == nullptr) {
        return false;
    }
    ZC_U32 key = (id << 16) | sid;
    // m_msgmap.insert(key, pMsg);
    auto res = m_msgmap.insert(std::make_pair(key, pMsg));
    return res.second;
}

ZC_S32 CMsgProcMod::MsgSubProc(zc_msg_t *sub, int size) {
    // if (!m_init) {
    //     return -1;
    // }

    LOG_TRACE("SubProc into, modid[%u], id[%hu],id[%hu]", m_modid, sub->id, sub->sid);
    ZC_U32 key = (sub->id << 16) | sub->sid;
    auto it = m_msgsubmap.find(key);
    if (it != m_msgsubmap.end()) {
        return it->second->MsgSubProc(sub, size);
    }

    return 0;
}

bool CMsgProcMod::registerMsgSub(ZC_U16 id, ZC_U16 sid, CMsgSubBase *pMsg) {
    if (m_init || pMsg == nullptr) {
        return false;
    }
    ZC_U32 key = (id << 16) | sid;
    // m_msgmap.insert(key, pMsg);
    auto res = m_msgsubmap.insert(std::make_pair(key, pMsg));
    return res.second;
}

bool CMsgProcMod::init() {
    if (m_init) {
        return false;
    }

    // check map
    if (m_msgmap.size() <= 0 && m_msgsubmap.size()) {
        LOG_ERROR("map size empty, init error");
        return false;
    }
    m_init = true;
    LOG_TRACE("init ok");
    return false;
}

bool CMsgProcMod::uninit() {
    if (!m_init) {
        return false;
    }

    for (auto it = m_msgmap.begin(); it != m_msgmap.end();) {
        ZC_SAFE_DELETE(it->second);
        it = m_msgmap.erase(it);
    }

    m_msgmap.clear();

    // subscribe msg
    for (auto it = m_msgsubmap.begin(); it != m_msgsubmap.end();) {
        ZC_SAFE_DELETE(it->second);
        it = m_msgsubmap.erase(it);
    }

    m_msgsubmap.clear();

    m_init = false;
    LOG_TRACE("uninit ok");
    return true;
}
}  // namespace zc
