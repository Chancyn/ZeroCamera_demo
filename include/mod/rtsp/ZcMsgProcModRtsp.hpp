// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"

#include "ZcModComm.hpp"
#include "ZcMsg.hpp"
#include "ZcMsgProcMod.hpp"

namespace zc {
// TODO(zhoucc): handle cmd type
typedef enum {
    RTSP_SMGR_HDL_CHG_NOTIFY_E = 0,    // chg notify
    RTSP_SMGR_HDL_GETINFO_E,
    RTSP_SMGR_HDL_SETINFO_E,           // setmgrinfo

    RTSP_SMGR_HDL_BUTT_E,
} rtsp_smgr_handle_e;

// TODO(zhoucc): handle cmd type
typedef enum {
    RTSP_MGR_HDL_RESTART_E = 0,

    RTSP_MGR_HDL_BUTT_E,
} rtsp_mgr_handle_e;

// streamMgr handle mod msg callback
typedef int (*RtspStreamMgrHandleMsgCb)(void *ptr, unsigned int type, void *indata, void *outdata);
// RtspManager handle mod msg callback
typedef int (*RtspMgrHandleMsgCb)(void *ptr, unsigned int type, void *indata, void *outdata);

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

 private:
    int m_init;
    rtsp_callback_info_t m_cbinfo;
};
}  // namespace zc
