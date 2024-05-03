// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"

#include "ZcMsg.hpp"
#include "ZcMsgMod.hpp"

namespace zc {
class CMsgModRtsp : public CMsgMod {
 public:
    CMsgModRtsp();
    virtual ~CMsgModRtsp();

 public:
    virtual bool Init();
    virtual bool Uninit();

    ZC_S32 MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 MsgRepProc(zc_msg_t *rep, int size);

 private:
    // Manager
    ZC_S32 _handleReqRtspManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspManVersion(zc_msg_t *rep, int size);
    ZC_S32 _handleReqRtspManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspManRestart(zc_msg_t *rep, int size);
    ZC_S32 _handleReqRtspManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepRtspManShutdown(zc_msg_t *rep, int size);
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
};
}  // namespace zc
