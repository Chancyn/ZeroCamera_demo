// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <string.h>

#include "zc_log.h"
#include "zc_msg.h"
#include "zc_msg_sys.h"
#include "zc_type.h"

#include "ZcMsg.hpp"
#include "ZcMsgProcMod.hpp"
#include "ZcType.hpp"
#include "sys/ZcMsgProcModSys.hpp"

namespace zc {
CMsgProcModSys::CMsgProcModSys() : CMsgProcMod(ZC_MODID_SYS_E, ZC_MID_SYS_BUTT), m_init(false) {
    LOG_TRACE("Constructor into");
}

CMsgProcModSys::~CMsgProcModSys() {
    LOG_TRACE("Destructor into");
    UnInit();
}

// Manager
ZC_S32 CMsgProcModSys::_handleReqSysManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManVersion,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManVersion(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManVersion,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManRegister,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManRegister(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManRegister,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManRestart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManRestart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManRestart,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManShutdown,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManShutdown(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManShutdown,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManKeepalive(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle _handleReqSysManKeepalive,iqsize[%d]", iqsize);
    zc_mod_keepalive_t *pkeepalive = reinterpret_cast<zc_mod_keepalive_t *>(req->data);
    LOG_TRACE("handle keepalive mid[%u] seqno[%u] status[%d]", pkeepalive->mid, pkeepalive->seqno,
              pkeepalive->status);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManKeepalive(zc_msg_t *rep, int size) {
    LOG_TRACE("handle _handleRepSysManKeepalive,size[%d]", size);
    zc_mod_keepalive_t *pkeepalive = reinterpret_cast<zc_mod_keepalive_t *>(rep->data);
    LOG_TRACE("handle keepalive mid[%u] seqno[%u] status[%d]", pkeepalive->mid, pkeepalive->seqno,
              pkeepalive->status);

    return 0;
}

// Time
ZC_S32 CMsgProcModSys::_handleReqSysTimeGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysTimeGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysTimeGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysTimeGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysTimeSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysTimeSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysTimeSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysTimeSet,size[%d]", size);

    return 0;
}

// Base
ZC_S32 CMsgProcModSys::_handleReqSysBaseGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysBaseGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysBaseGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysBaseGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysBaseSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysBaseSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysBaseSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysBaseSet,size[%d]", size);

    return 0;
}

// User
ZC_S32 CMsgProcModSys::_handleReqSysUserGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUserGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUserGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUserGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysUserSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUserSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUserSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUserSet,size[%d]", size);

    return 0;
}

// upgrade
ZC_S32 CMsgProcModSys::_handleReqSysUpgStart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUpgStart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUpgStart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUpgStart,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysUpgStop(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUpgStop,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUpgStop(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUpgStop,size[%d]", size);

    return 0;
}

bool CMsgProcModSys::Init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // TODO(zhoucc) register all msgfunction
    // ZC_MID_SYS_MAN_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_REGISTER_E, &CMsgProcModSys::_handleReqSysManRegister,
                 &CMsgProcModSys::_handleRepSysManRegister);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_VERSION_E, &CMsgProcModSys::_handleReqSysManVersion,
                 &CMsgProcModSys::_handleRepSysManVersion);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_RESTART_E, &CMsgProcModSys::_handleReqSysManRestart,
                 &CMsgProcModSys::_handleRepSysManRestart);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_SHUTDOWN_E, &CMsgProcModSys::_handleReqSysManShutdown,
                 &CMsgProcModSys::_handleRepSysManShutdown);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_KEEPALIVE_E, &CMsgProcModSys::_handleReqSysManKeepalive,
                 &CMsgProcModSys::_handleRepSysManKeepalive);

    // ZC_MID_SYS_TIME_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_TIME_E, ZC_MSID_SYS_TIME_GET_E, &CMsgProcModSys::_handleReqSysTimeGet,
                 &CMsgProcModSys::_handleRepSysTimeGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_TIME_E, ZC_MSID_SYS_TIME_SET_E, &CMsgProcModSys::_handleReqSysTimeSet,
                 &CMsgProcModSys::_handleRepSysTimeSet);

    // ZC_MID_SYS_BASE_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_BASE_E, ZC_MSID_SYS_BASE_GET_E, &CMsgProcModSys::_handleReqSysBaseGet,
                 &CMsgProcModSys::_handleRepSysBaseGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_BASE_E, ZC_MSID_SYS_BASE_SET_E, &CMsgProcModSys::_handleReqSysBaseSet,
                 &CMsgProcModSys::_handleRepSysBaseSet);

    // ZC_MID_SYS_USER_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_USER_E, ZC_MSID_SYS_USER_GET_E, &CMsgProcModSys::_handleReqSysUserGet,
                 &CMsgProcModSys::_handleRepSysUserGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_USER_E, ZC_MSID_SYS_USER_SET_E, &CMsgProcModSys::_handleReqSysUserSet,
                 &CMsgProcModSys::_handleRepSysUserSet);

    // ZC_MID_SYS_UPG_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_UPG_E, ZC_MSID_SYS_UPG_START_E, &CMsgProcModSys::_handleReqSysUpgStart,
                 &CMsgProcModSys::_handleRepSysUpgStart);
    REGISTER_MSG(m_modid, ZC_MID_SYS_UPG_E, ZC_MSID_SYS_UPG_STOP_E, &CMsgProcModSys::_handleReqSysUpgStop,
                 &CMsgProcModSys::_handleRepSysUpgStop);
    init();
    m_init = true;

    LOG_TRACE("Init ok");
    return true;
}

bool CMsgProcModSys::UnInit() {
    if (!m_init) {
        LOG_ERROR("not init");
        return false;
    }

    uninit();
    m_init = false;
    LOG_TRACE("UnInit ok");
    return true;
}

}  // namespace zc
