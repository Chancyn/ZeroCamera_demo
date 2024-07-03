// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_mod_base.h"
#include "zc_msg.h"
#include "zc_type.h"

#include "MsgCommRepServer.hpp"
#include "MsgCommReqClient.hpp"
#include "Thread.hpp"
#include "ZcModCli.hpp"
#include "ZcModComm.hpp"
#include "ZcMsgProcMod.hpp"

namespace zc {
#define ZC_MOD_KIEEPALIVE_TIME (10)  // keepalive msg timeout

// register status
typedef enum {
    MODCLI_STATUS_EXPIRED_E = -2,  // expired keepalive
    MODCLI_STATUS_REG_ERR_E = -1,  // register error
    MODCLI_STATUS_UNREGISTER_E,    // unsigned register success
    MODCLI_STATUS_REGISTERED_E,    // register success

    MODCLI_STATUS_BUTT_E,
} modcli_status_e;

//  status
typedef struct {
    ZC_U32 modid;     // mod id
    ZC_S32 pid;       // pid
    ZC_U32 regtime;   // last regtime
    ZC_U32 lasttime;  // last keepalive time
    ZC_S32 status;    // status modcli_status_e
    ZC_CHAR url[ZC_URL_SIZE];     // process name
    ZC_CHAR pname[ZC_MAX_PNAME];  // process name
} sys_modcli_status_t;

class CModBase : public CModComm, public CModReqCli, public Thread {
 public:
    explicit CModBase(ZC_U8 modid, ZC_U32 version = ZC_MSG_VERSION);
    virtual ~CModBase();
    virtual bool Init(void *cbinfo) = 0;
    virtual bool UnInit() = 0;
    static void DumpModMsg(const zc_msg_t &msg);

 protected:
    void BuildRepMsgHdr(zc_msg_t *rep, zc_msg_t *req);
    zc_msg_errcode_e MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    zc_msg_errcode_e MsgSubProc(zc_msg_t *sub, int iqsize);
    bool registerMsgProcMod(CMsgProcMod *msgprocmod);
    bool unregisterMsgProcMod(CMsgProcMod *msgprocmod);
    bool initReqSvr(MsgCommReqSerHandleCb svrcb);
    bool unInitReqSvr();
    void _initlicense();
    int _checklicense();
    virtual int modprocess() = 0;

 private:
    // first init license
    virtual int process();

 private:
    bool m_init;
    ZC_U32 m_expire;                  // license expire time
    ZC_U32 m_inittime;                // license load time
    sys_lic_status_e m_syslicstatus;  // license status

 protected:
    ZC_CHAR m_url[ZC_URL_SIZE];
    ZC_CHAR m_pname[ZC_MAX_PNAME];
    // msg handle
    CMsgProcMod *m_pmsgmodproc;
};
}  // namespace zc
