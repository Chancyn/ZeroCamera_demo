// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"
#include "zc_basic_fun.h"

#include "ZcModPubSub.hpp"

namespace zc {
// pub tab
static const char *g_Modpuburltab[] = {
    ZC_SYS_URL_PUB,
    ZC_CODEC_URL_PUB,
    ZC_RTSP_URL_PUB,
};

static inline const char *get_puburl_bymodid(ZC_U8 modid) {
    if (modid >= ZC_MODID_BUTT) {
        return nullptr;
    }
    modid %= _SIZEOFTAB(g_Modpuburltab);
    return g_Modpuburltab[modid];
}

CModPublish::CModPublish(ZC_U8 modid) : m_pubmodid(modid) {
    m_pid = getpid();
    strncpy(m_puburl, get_puburl_bymodid(m_pubmodid), sizeof(m_puburl) - 1);
}

CModPublish::~CModPublish() {}

bool CModPublish::InitPub() {
    return Open(m_puburl);
}

bool CModPublish::BuildSubMsgHdr(zc_msg_t *pmsg, ZC_U16 id, ZC_U16 sid, ZC_U8 chn, ZC_U32 size) {
    if (!pmsg) {
        pmsg = reinterpret_cast<zc_msg_t *>(new char[sizeof(zc_msg_t) + size]());
    }
    pmsg->pid = m_pid;
    pmsg->modid = m_pubmodid;
    pmsg->ver = ZC_MSG_VERSION;
    pmsg->modidto = 0;
    pmsg->msgtype = ZC_MSG_TYPE_PUB_E;
    pmsg->chn = chn;
    pmsg->id = id;
    pmsg->sid = sid;
    pmsg->size = size;
    pmsg->err = 0;
    pmsg->ts = zc_system_time();

    return true;
}

bool CModPublish::Publish(void *buf, size_t len) {
    return Send(buf, len);
}

/************************************************************************/
CModSubscriber::CModSubscriber(ZC_U8 modid) : m_pubmodid(modid) {
    strncpy(m_puburl, get_puburl_bymodid(m_pubmodid), sizeof(m_puburl) - 1);
}

CModSubscriber::~CModSubscriber() {}

bool CModSubscriber::InitSub(MsgCommSubCliHandleCb handle) {
    return Open(m_puburl, handle);
}

}  // namespace zc
