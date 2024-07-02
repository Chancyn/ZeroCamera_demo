// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"
#include "zc_rtsp_mgr_handle.h"
#include "zc_rtsp_smgr_handle.h"

#include "ZcModComm.hpp"
#include "ZcMsg.hpp"
#include "ZcMsgProcMod.hpp"

namespace zc {
typedef struct {
    RtspStreamMgrHandleMsgCb streamMgrHandleCb;
    void *streamMgrContext;
    RtspMgrHandleMsgCb MgrHandleCb;
    void *MgrContext;
} rtsp_callback_info_t;

class CMsgProcModRtsp : public CMsgProcMod {
 public:
    CMsgProcModRtsp();
    virtual ~CMsgProcModRtsp();

 public:
    virtual bool Init(void *cbinfo);
    virtual bool UnInit();

 private:
    bool _unInit();

 private:
    // Manager
    ZC_S32 _handleReqRtspManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspManVersion(zc_msg_t *rep, int size);
    ZC_S32 _handleReqRtspManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspManRestart(zc_msg_t *rep, int size);
    ZC_S32 _handleReqRtspManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspManShutdown(zc_msg_t *rep, int size);

    // SMgr
    ZC_S32 _handleReqRtspSMgrChgNotify(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspSMgrChgNotify(zc_msg_t *rep, int size);

    // Cfg
    ZC_S32 _handleReqRtspCfgGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspCfgGet(zc_msg_t *rep, int size);
    ZC_S32 _handleReqRtspCfgSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspCfgSet(zc_msg_t *rep, int size);

    // CtrlReqIDR
    ZC_S32 _handleReqRtspCtrlReqIDR(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspCtrlReqIDR(zc_msg_t *rep, int size);

    /*********************************************************************** */
    // subscribe
    ZC_S32 _handleSubManReg(zc_msg_t *sub, int size);
    ZC_S32 _handleSubManStreamUpdate(zc_msg_t *sub, int size);

 private:
    int m_init;
    rtsp_callback_info_t m_cbinfo;
};
}  // namespace zc
