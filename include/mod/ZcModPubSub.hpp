// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_mod_base.h"
#include "zc_msg.h"

#include "MsgCommPubServer.hpp"
#include "MsgCommSubClient.hpp"

namespace zc {
// publish & subscribers
class CModPublish : public CMsgCommPubServer {
 public:
    explicit CModPublish(ZC_U8 modid);
    virtual ~CModPublish();
    bool InitPub();
    bool BuildSubMsgHdr(zc_msg_t *pmsg, ZC_U16 id, ZC_U16 sid, ZC_U8 chn, ZC_U32 size);
    bool Publish(void *buf, size_t len);

 private:
    ZC_S32 m_pid;
    ZC_U8 m_pubmodid;
    ZC_CHAR m_puburl[ZC_URL_SIZE];
};

class CModSubscriber : public CMsgCommSubClient {
 public:
    explicit CModSubscriber(ZC_U8 modid);
    virtual ~CModSubscriber();
    bool InitSub(MsgCommSubCliHandleCb handle);

 private:
    ZC_U8 m_pubmodid;
    ZC_CHAR m_puburl[ZC_URL_SIZE];
};
}  // namespace zc
