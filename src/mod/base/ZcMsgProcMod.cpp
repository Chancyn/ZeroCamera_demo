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
    if (unlikely(_checklicense() < 0)) {
        return -1;
    }
    LOG_TRACE("MsgReqProc into, pid:%d,modid:%u, id[%hu],id[%hu]", req->pid, req->modid, req->id, req->sid);
    ZC_U32 key = (req->id << 16) | req->sid;
    auto it = m_msgmap.find(key);
    if (it != m_msgmap.end()) {
        it->second->MsgReqProc(req, iqsize, rep, opsize);
    }

    return 0;
}

ZC_S32 CMsgProcMod::MsgRepProc(zc_msg_t *rep, int size) {
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

bool CMsgProcMod::registerMsg(ZC_U16 id, ZC_U16 sid, CMsgBase *pMsg) {
    if (m_init || pMsg == nullptr) {
        return false;
    }
    ZC_U32 key = (id << 16) | sid;
    // m_msgmap.insert(key, pMsg);
    auto res = m_msgmap.insert(std::make_pair(key, pMsg));
    return res.second;
}

bool CMsgProcMod::init() {
    if (m_init) {
        return false;
    }

    // init license
    _initlicense();

    // check map
    if (m_msgmap.size() <= 0) {
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
    m_init = false;
    LOG_TRACE("uninit ok");
    return true;
}

int CMsgProcMod::_checklicense() {
    if (unlikely(m_syslicstatus == SYS_LIC_STATUS_TEMP_LIC_E)) {
        time_t now = time(NULL);
        if (now > m_expire) {
            m_syslicstatus = SYS_LIC_STATUS_EXPIRED_LIC_E;
            LOG_ERROR("license timeout now:%u > %u", now, m_expire);
        }
    }

    return m_syslicstatus;
}
void CMsgProcMod::_initlicense() {
    // TODO(zhoucc): load license
    m_inittime = time(NULL);
    m_syslicstatus = SYS_LIC_STATUS_SUC_E;
    m_expire = m_inittime + ZC_MOD_LIC_EXPIRE_TIME;
    LOG_TRACE("modid:%d init license:%d", m_modid, m_syslicstatus);
    ZC_ASSERT(m_syslicstatus != SYS_LIC_STATUS_ERR_E);

    return;
}
}  // namespace zc
