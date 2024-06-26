// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <list>
#include "zc_mod_base.h"
#include "zc_type.h"
#include "zc_sys_mgr_handle.h"
#include "zc_sys_smgr_handle.h"

#include "ZcMsg.hpp"
#include "ZcMsgProcMod.hpp"

namespace zc {

typedef struct {
    SysStreamMgrHandleMsgCb streamMgrHandleCb;
    void *streamMgrContext;
    SysMgrHandleMsgCb MgrHandleCb;
    void *MgrContext;
} sys_callback_info_t;

class CMsgProcModSys : public CMsgProcMod {
 public:
    CMsgProcModSys();
    virtual ~CMsgProcModSys();

 public:
    virtual bool Init(void *cbinfo);
    virtual bool UnInit();

    ZC_S32 MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 MsgRepProc(zc_msg_t *rep, int size);

 private:
    ZC_S32 _handleRepNull(zc_msg_t *rep, int size);
    // Manager
    ZC_S32 _handleReqSysManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysManVersion(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysManRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysManRegister(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysManRestart(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysManShutdown(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysManKeepalive(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysManKeepalive(zc_msg_t *rep, int size);

    // SMgr StreamMgr
    ZC_S32 _handleReqSysSMgrRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysSMgrRegister(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysSMgrUnRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysSMgrUnRegister(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysSMgrGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysSMgrGet(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysSMgrSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysSMgrSet(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysSMgrGetAll(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);

    // Time
    ZC_S32 _handleReqSysTimeGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysTimeGet(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysTimeSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysTimeSet(zc_msg_t *rep, int size);
    // Base
    ZC_S32 _handleReqSysBaseGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysBaseGet(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysBaseSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysBaseSet(zc_msg_t *rep, int size);
    // User
    ZC_S32 _handleReqSysUserGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysUserGet(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysUserSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysUserSet(zc_msg_t *rep, int size);

    // upgrade
    ZC_S32 _handleReqSysUpgStart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysUpgStart(zc_msg_t *rep, int size);
    ZC_S32 _handleReqSysUpgStop(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 _handleRepSysUpgStop(zc_msg_t *rep, int size);

 private:
    int m_init;
    sys_callback_info_t m_cbinfo;
};
}  // namespace zc
