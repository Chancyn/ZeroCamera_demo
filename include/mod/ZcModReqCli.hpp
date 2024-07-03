// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <memory>

#include "zc_mod_base.h"
#include "zc_msg.h"
#include "zc_type.h"
#include "zc_frame.h"

#include "MsgCommReqClient.hpp"

namespace zc {

class CModReqCli  {
 public:
    explicit CModReqCli(ZC_U8 modid, ZC_U32 version = ZC_MSG_VERSION);
    virtual ~CModReqCli();
    bool BuildReqMsgHdr(zc_msg_t *pmsg, ZC_U8 modid, ZC_U16 id, ZC_U16 sid, ZC_U8 chn, ZC_U32 size);
    bool MsgSendTo(zc_msg_t *pmsg, const char *urlto, zc_msg_t *prmsg, size_t *buflen);
    bool MsgSendTo(zc_msg_t *pmsg, zc_msg_t *prmsg, size_t *buflen);
    static const char *GetUrlbymodid(ZC_U8 modid);
    int sendSMgrGetInfo(unsigned int type, unsigned int chn, zc_stream_info_t *info);
    int sendSMgrSetInfo(unsigned int type, unsigned int chn, zc_stream_info_t *info);

 protected:
    ZC_S32 m_pid;
    ZC_U8 m_modid;
    ZC_U32 m_seqno;
    ZC_U32 m_version;
    ZC_CHAR m_url[ZC_URL_SIZE];
    // ZC_CHAR m_name[ZC_MODNAME_SIZE];
};
}  // namespace zc
