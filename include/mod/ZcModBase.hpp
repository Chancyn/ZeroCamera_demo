// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <memory>

#include "zc_mod_base.h"
#include "zc_msg.h"
#include "zc_type.h"

#include "MsgCommRepServer.hpp"
#include "MsgCommReqClient.hpp"
#include "Thread.hpp"
#include "ZcModComm.hpp"
#include "ZcMsgProcMod.hpp"

namespace zc {
#define ZC_MOD_KIEEPALIVE_TIME (10)  // keepalive msg timeout

// register status
typedef enum {
    MODCLI_STATUS_EXPIRED_E = -2,  // expired keepalive
    MODCLI_STATUS_REG_ERR_E = -1,  // register error
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

class CModBase : public CModComm, public Thread {
 public:
    explicit CModBase(ZC_U8 modid, ZC_U32 version = ZC_MSG_VERSION);
    virtual ~CModBase();
    virtual bool Init(void *cbinfo) = 0;
    virtual bool UnInit() = 0;
    bool BuildReqMsgHdr(zc_msg_t *pmsg, ZC_U8 modid, ZC_U16 id, ZC_U16 sid, ZC_U8 chn, ZC_U32 size);
    bool MsgSendTo(zc_msg_t *pmsg, const char *urlto, zc_msg_t *prmsg, size_t *buflen);
    bool MsgSendTo(zc_msg_t *pmsg, zc_msg_t *prmsg, size_t *buflen);
    const char *GetUrlbymodid(ZC_U8 modid);
    virtual int process();

 protected:
    bool registerMsgProcMod(CMsgProcMod *msgprocmod);
    bool unregisterMsgProcMod(CMsgProcMod *msgprocmod);
    bool init();
    bool unInit();

 private:
    bool registerInsert(zc_msg_t *msg);
    bool unregisterRemove(zc_msg_t *msg);
    bool updateStatus(zc_msg_t *msg);
    ZC_S32 _svrSysRecvReqProc(char *req, int iqsize, char *rep, int *opsize);
    ZC_S32 _svrRecvReqProc(char *req, int iqsize, char *rep, int *opsize);
    ZC_S32 _cliRecvRepProc(char *rep, int size);

    int updateStatus(ZC_S32 pid);
    int _sysCheckModCliStatus();
    int _process_sys();
    int _process_mod();
    int _sendRegisterMsg(int cmd);
    int _sendKeepaliveMsg();

 private:
    bool m_init;
    int m_status;
    ZC_S32 m_pid;
    ZC_U8 m_modid;
    ZC_U32 m_seqno;
    ZC_U32 m_version;
    ZC_CHAR m_url[ZC_URL_SIZE];
    ZC_CHAR m_name[ZC_MODNAME_SIZE];
    ZC_CHAR m_pname[ZC_MAX_PNAME];
    // msg handle
    CMsgProcMod *m_pmsgmodproc;

    // ZC_U64 = (pid << 32) | modid;
    std::map<ZC_U64, std::shared_ptr<sys_modcli_status_t>> m_modmap;  // modclimap
    std::mutex m_mutex;                                               // map lock
};
}  // namespace zc
