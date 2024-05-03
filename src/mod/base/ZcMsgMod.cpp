// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <string.h>

#include <map>
#include <utility>

#include "zc_log.h"
#include "zc_type.h"

#include "ZcType.hpp"

#include "ZcMsg.hpp"
#include "ZcMsgMod.hpp"

namespace zc {
CMsgMod::CMsgMod(ZC_U8 modid, ZC_U16 idmax) : m_modid(modid), m_idmax(idmax), m_init(false) {
    LOG_TRACE("Constructor into, modid[%u] msgidmax[%u]", modid, idmax);
}

CMsgMod::~CMsgMod() {
    LOG_TRACE("Destructor into");
    uninit();
}

ZC_S32 CMsgMod::MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("MsgReqProc into, modid[%u], id[%hu],id[%hu]", m_modid, rep->id, rep->sid);
    ZC_U32 key = (rep->id << 16) | rep->sid;
    auto it = m_msgmap.find(key);
    if (it != m_msgmap.end()) {
        it->second->MsgReqProc(req, iqsize, rep, opsize);
    }

    return 0;
}

ZC_S32 CMsgMod::MsgRepProc(zc_msg_t *rep, int size) {
    if (m_init) {
        return -1;
    }

    LOG_TRACE("RepProc into, modid[%u], id[%hu],id[%hu]", m_modid, rep->id, rep->sid);
    ZC_U32 key = (rep->id << 16) | rep->sid;
    auto it = m_msgmap.find(key);
    if (it != m_msgmap.end()) {
        it->second->MsgRepProc(rep, size);
    }

    return 0;
}

ZC_S32 CMsgMod::registerMsg(ZC_U16 id, ZC_U16 sid, CMsgBase *pMsg) {
    if (m_init || pMsg == nullptr) {
        return false;
    }
    ZC_U32 key = (id << 16) | sid;
    // m_msgmap.insert(key, pMsg);
    m_msgmap.insert(std::make_pair(key, pMsg));

    return true;
}

bool CMsgMod::init() {
    if (m_init) {
        return false;
    }

    // check map
    if (m_msgmap.size() <= 0) {
        LOG_ERROR("map size empty, init error");
        return false;
    }
    m_init = true;
    LOG_TRACE("init ok");
    return false;
}

bool CMsgMod::uninit() {
    if (!m_init) {
        return false;
    }

    for (auto it = m_msgmap.begin(); it != m_msgmap.end();) {
        ZC_SAFE_DELETE(it->second);
        it = m_msgmap.erase(it);
    }

    m_msgmap.clear();
    LOG_TRACE("uninit ok");
    return true;
}
}  // namespace zc
