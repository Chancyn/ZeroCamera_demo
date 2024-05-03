// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <string.h>

#include "zc_log.h"
#include "zc_msg.h"
#include "zc_msg_sys.h"
#include "zc_msg_rtsp.h"
#include "zc_type.h"

#include "ZcMsg.hpp"
#include "ZcMsgMod.hpp"
#include "ZcMsgModRtsp.hpp"
#include "ZcType.hpp"

namespace zc {
CMsgModRtsp::CMsgModRtsp() : CMsgMod(ZC_MODID_RTSP_E, ZC_MID_RTSP_BUTT), m_init(false) {
    LOG_TRACE("Constructor into");
}

CMsgModRtsp::~CMsgModRtsp() {
    LOG_TRACE("Destructor into");
    Uninit();
}

// Manager
ZC_S32 CMsgModRtsp::_handleReqRtspManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspManVersion,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleRepRtspManVersion(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspManVersion,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleReqRtspManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspManRestart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleRepRtspManRestart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspManRestart,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleReqRtspManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspManShutdown,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleRepRtspManShutdown(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspManShutdown,size[%d]", size);

    return 0;
}

// Cfg
ZC_S32 CMsgModRtsp::_handleReqRtspCfgGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspCfgGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleRepRtspCfgGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspCfgGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleReqRtspCfgSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspCfgSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleRepRtspCfgSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspCfgSet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleReqRtspCtrlReqIDR(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspCtrlReqIDR,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgModRtsp::_handleRepRtspCtrlReqIDR(zc_msg_t *rep, int size) {
    LOG_TRACE("handle ReqRtspCtrlReqIDR,size[%d]", size);

    return 0;
}

bool CMsgModRtsp::Init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // TODO(zhoucc) register all msgfunction
    // ZC_MID_RTSP_MAN_E
    // REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_REGISTER_E, &CMsgModRtsp::_handleReqRtspManRegister,
    //              &CMsgModRtsp::_handleRepRtspManRegister);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_VERSION_E, &CMsgModRtsp::_handleReqRtspManVersion,
                 &CMsgModRtsp::_handleRepRtspManVersion);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_RESTART_E, &CMsgModRtsp::_handleReqRtspManRestart,
                 &CMsgModRtsp::_handleRepRtspManRestart);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_SHUTDOWN_E, &CMsgModRtsp::_handleReqRtspManShutdown,
                 &CMsgModRtsp::_handleRepRtspManShutdown);

    // ZC_MID_RTSP_CFG_E
    REGISTER_MSG(m_modid, ZC_MID_RTSP_CFG_E, ZC_MID_RTSP_CFG_E, &CMsgModRtsp::_handleReqRtspCfgGet,
                 &CMsgModRtsp::_handleRepRtspCfgGet);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_CFG_E, ZC_MID_RTSP_CFG_E, &CMsgModRtsp::_handleReqRtspCfgSet,
                 &CMsgModRtsp::_handleRepRtspCfgSet);

    // ZC_MID_RTSP_CTRL_E
    REGISTER_MSG(m_modid, ZC_MID_RTSP_CTRL_E, ZC_MSID_RTSP_REQIDR_E, &CMsgModRtsp::_handleReqRtspCtrlReqIDR,
                 &CMsgModRtsp::_handleRepRtspCtrlReqIDR);

    init();
    m_init = true;

    LOG_ERROR("init ok");
    return true;
}

bool CMsgModRtsp::Uninit() {
    if (!m_init) {
        LOG_ERROR("not init");
        return false;
    }

    m_init = false;
    uninit();
    return true;
}

}  // namespace zc
