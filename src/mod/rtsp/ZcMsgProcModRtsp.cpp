// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <string.h>

#include "zc_log.h"
#include "zc_msg.h"
#include "zc_msg_rtsp.h"
#include "zc_msg_sys.h"
#include "zc_type.h"

#include "ZcMsg.hpp"
#include "ZcMsgProcMod.hpp"
#include "ZcType.hpp"
#include "rtsp/ZcMsgProcModRtsp.hpp"

namespace zc {
CMsgProcModRtsp::CMsgProcModRtsp() : CMsgProcMod(ZC_MODID_RTSP_E, ZC_MID_RTSP_BUTT), m_init(false) {
    LOG_TRACE("Constructor into");
}

CMsgProcModRtsp::~CMsgProcModRtsp() {
    LOG_TRACE("Destructor into");
    UnInit();
}

// Manager
ZC_S32 CMsgProcModRtsp::_handleReqRtspManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspManVersion,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleRepRtspManVersion(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspManVersion,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleReqRtspManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspManRestart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleRepRtspManRestart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspManRestart,size[%d]", size);

    if (m_cbinfo.MgrHandleCb) {
        m_cbinfo.MgrHandleCb(m_cbinfo.MgrContext, RTSP_MGR_HDL_RESTART_E, nullptr, nullptr);
    }

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleReqRtspManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspManShutdown,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleRepRtspManShutdown(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspManShutdown,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleReqRtspSMgrChgNotify(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    zc_mod_smgrcli_chgnotify_t *pReqMsg = reinterpret_cast<zc_mod_smgrcli_chgnotify_t *>(req->data);
    LOG_TRACE("handle ReqRtspSMgrNotify,iqsize:%d,modid:%d ", iqsize, pReqMsg->modid);
    if (m_cbinfo.streamMgrHandleCb) {
        m_cbinfo.streamMgrHandleCb(m_cbinfo.streamMgrContext, RTSP_SMGR_HDL_CHG_NOTIFY_E, nullptr, nullptr);
    }

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleRepRtspSMgrChgNotify(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspSMgrNotify,size[%d]", size);

    return 0;
}

// Cfg
ZC_S32 CMsgProcModRtsp::_handleReqRtspCfgGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspCfgGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleRepRtspCfgGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspCfgGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleReqRtspCfgSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspCfgSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleRepRtspCfgSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepRtspCfgSet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleReqRtspCtrlReqIDR(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqRtspCtrlReqIDR,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModRtsp::_handleRepRtspCtrlReqIDR(zc_msg_t *rep, int size) {
    LOG_TRACE("handle ReqRtspCtrlReqIDR,size[%d]", size);

    return 0;
}

bool CMsgProcModRtsp::Init(void *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // TODO(zhoucc) register all msgfunction
    // ZC_MID_RTSP_MAN_E
    // REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_REGISTER_E,
    // &CMsgProcModRtsp::_handleReqRtspManRegister,
    //              &CMsgProcModRtsp::_handleRepRtspManRegister);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_VERSION_E, &CMsgProcModRtsp::_handleReqRtspManVersion,
                 &CMsgProcModRtsp::_handleRepRtspManVersion);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_RESTART_E, &CMsgProcModRtsp::_handleReqRtspManRestart,
                 &CMsgProcModRtsp::_handleRepRtspManRestart);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_MAN_E, ZC_MSID_RTSP_MAN_SHUTDOWN_E, &CMsgProcModRtsp::_handleReqRtspManShutdown,
                 &CMsgProcModRtsp::_handleRepRtspManShutdown);

    // ZC_MID_RTSP_STREAMMGR_E
    REGISTER_MSG(m_modid, ZC_MID_RTSP_SMGRCLI_E, ZC_MSID_SMGRCLI_CHGNOTIFY_E, &CMsgProcModRtsp::_handleReqRtspSMgrChgNotify,
                 &CMsgProcModRtsp::_handleRepRtspSMgrChgNotify);

    // ZC_MID_RTSP_CFG_E
    REGISTER_MSG(m_modid, ZC_MID_RTSP_CFG_E, ZC_MSID_RTSP_CFG_GET_E, &CMsgProcModRtsp::_handleReqRtspCfgGet,
                 &CMsgProcModRtsp::_handleRepRtspCfgGet);
    REGISTER_MSG(m_modid, ZC_MID_RTSP_CFG_E, ZC_MSID_RTSP_CFG_SET_E, &CMsgProcModRtsp::_handleReqRtspCfgSet,
                 &CMsgProcModRtsp::_handleRepRtspCfgSet);

    // ZC_MID_RTSP_CTRL_E
    REGISTER_MSG(m_modid, ZC_MID_RTSP_CTRL_E, ZC_MSID_RTSP_REQIDR_E, &CMsgProcModRtsp::_handleReqRtspCtrlReqIDR,
                 &CMsgProcModRtsp::_handleRepRtspCtrlReqIDR);

    init();

    if (cbinfo) {
        memcpy(&m_cbinfo, cbinfo, sizeof(m_cbinfo));
    }

    m_init = true;

    LOG_TRACE("init ok");
    return true;
}

bool CMsgProcModRtsp::UnInit() {
    if (!m_init) {
        LOG_ERROR("not init");
        return false;
    }

    uninit();
    m_init = false;
    return true;
}

}  // namespace zc
