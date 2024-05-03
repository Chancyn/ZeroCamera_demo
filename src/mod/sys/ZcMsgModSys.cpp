// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <string.h>

#include "zc_log.h"
#include "zc_msg.h"
#include "zc_msg_sys.h"
#include "zc_type.h"

#include "ZcMsg.hpp"
#include "ZcMsgMod.hpp"
#include "ZcMsgModSys.hpp"
#include "ZcType.hpp"

namespace zc {
CMsgModSys::CMsgModSys() : CMsgMod(ZC_MODID_SYS_E, ZC_MID_SYS_BUTT), m_init(false) {
    LOG_TRACE("Constructor into");
}

CMsgModSys::~CMsgModSys() {
    LOG_TRACE("Destructor into");
    Uninit();
}

// Manager
ZC_S32 CMsgModSys::_handleReqSysManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManVersion,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysManVersion(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManVersion,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModSys::_handleReqSysManRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManRegister,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysManRegister(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManRegister,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModSys::_handleReqSysManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManRestart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysManRestart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManRestart,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModSys::_handleReqSysManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManShutdown,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysManShutdown(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManShutdown,size[%d]", size);

    return 0;
}

// Time
ZC_S32 CMsgModSys::_handleReqSysTimeGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysTimeGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysTimeGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysTimeGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModSys::_handleReqSysTimeSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysTimeSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysTimeSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysTimeSet,size[%d]", size);

    return 0;
}

// Base
ZC_S32 CMsgModSys::_handleReqSysBaseGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysBaseGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysBaseGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysBaseGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModSys::_handleReqSysBaseSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysBaseSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysBaseSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysBaseSet,size[%d]", size);

    return 0;
}

// User
ZC_S32 CMsgModSys::_handleReqSysUserGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUserGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysUserGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUserGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModSys::_handleReqSysUserSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUserSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysUserSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUserSet,size[%d]", size);

    return 0;
}

// upgrade
ZC_S32 CMsgModSys::_handleReqSysUpgStart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUpgStart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysUpgStart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUpgStart,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModSys::_handleReqSysUpgStop(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUpgStop,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModSys::_handleRepSysUpgStop(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUpgStop,size[%d]", size);

    return 0;
}

bool CMsgModSys::Init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // TODO(zhoucc) register all msgfunction
    // ZC_MID_SYS_MAN_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_REGISTER_E, &CMsgModSys::_handleReqSysManRegister,
                 &CMsgModSys::_handleRepSysManRegister);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_VERSION_E, &CMsgModSys::_handleReqSysManVersion,
                 &CMsgModSys::_handleRepSysManVersion);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_RESTART_E, &CMsgModSys::_handleReqSysManRestart,
                 &CMsgModSys::_handleRepSysManRestart);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_SHUTDOWN_E, &CMsgModSys::_handleReqSysManShutdown,
                 &CMsgModSys::_handleRepSysManShutdown);

    // ZC_MID_SYS_TIME_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_TIME_E, ZC_MSID_SYS_TIME_GET_E, &CMsgModSys::_handleReqSysTimeGet,
                 &CMsgModSys::_handleRepSysTimeGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_TIME_E, ZC_MSID_SYS_TIME_SET_E, &CMsgModSys::_handleReqSysTimeSet,
                 &CMsgModSys::_handleRepSysTimeSet);

    // ZC_MID_SYS_BASE_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_BASE_E, ZC_MSID_SYS_BASE_GET_E, &CMsgModSys::_handleReqSysBaseGet,
                 &CMsgModSys::_handleRepSysBaseGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_BASE_E, ZC_MSID_SYS_BASE_SET_E, &CMsgModSys::_handleReqSysBaseSet,
                 &CMsgModSys::_handleRepSysBaseSet);

    // ZC_MID_SYS_USER_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_USER_E, ZC_MSID_SYS_USER_GET_E, &CMsgModSys::_handleReqSysUserGet,
                 &CMsgModSys::_handleRepSysUserGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_USER_E, ZC_MSID_SYS_USER_SET_E, &CMsgModSys::_handleReqSysUserSet,
                 &CMsgModSys::_handleRepSysUserSet);

    // ZC_MID_SYS_UPG_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_UPG_E, ZC_MSID_SYS_UPG_START_E, &CMsgModSys::_handleReqSysUpgStart,
                 &CMsgModSys::_handleRepSysUpgStart);
    REGISTER_MSG(m_modid, ZC_MID_SYS_UPG_E, ZC_MSID_SYS_UPG_STOP_E, &CMsgModSys::_handleReqSysUpgStop,
                 &CMsgModSys::_handleRepSysUpgStop);
    init();
    m_init = true;

    LOG_ERROR("init ok");
    return true;
}

bool CMsgModSys::Uninit() {
    if (!m_init) {
        LOG_ERROR("not init");
        return false;
    }

    m_init = false;
    uninit();
    return true;
}

}  // namespace zc
