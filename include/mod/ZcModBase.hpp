// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_mod_base.h"
#include "zc_msg.h"
#include "zc_type.h"

#include "MsgCommRepServer.hpp"
#include "MsgCommReqClient.hpp"
#include "Thread.hpp"
#include "ZcModComm.hpp"
#include "ZcMsgProcMod.hpp"

namespace zc {
class CModBase : public CModComm, public Thread {
 public:
    CModBase(ZC_U8 modid);
    virtual ~CModBase();
    virtual bool Init() = 0;
    virtual bool UnInit() = 0;
    bool BuildReqMsgHdr(zc_msg_t *pmsg, ZC_U8 modid, ZC_U16 id, ZC_U16 sid, ZC_U8 chn, ZC_U32 size);
    bool MsgSendTo(zc_msg_t *pmsg, const char *urlto);
    bool MsgSendTo(zc_msg_t *pmsg);
    const char *GetUrlbymodid(ZC_U8 modid);
    virtual int process();

 protected:
    bool registerMsgProcMod(CMsgProcMod *msgprocmod);
    bool unregisterMsgProcMod(CMsgProcMod *msgprocmod);
    bool init();
    bool unInit();

 private:
    ZC_S32 _svrRecvReqProc(char *req, int iqsize, char *rep, int *opsize);
    ZC_S32 _cliRecvRepProc(char *rep, int size);
    int _process_sys();
    int _process_mod();
    int _sendRegisterMsg();
   int _sendKeepaliveMsg();

 private:
    bool m_init;
    int m_status;
    ZC_U8 m_modid;
    ZC_U32 m_seqno;
    ZC_CHAR m_url[ZC_URL_SIZE];
    ZC_CHAR m_name[ZC_MODNAME_SIZE];
    // msg handle
    CMsgProcMod *m_pmsgmodproc;

};
}  // namespace zc
