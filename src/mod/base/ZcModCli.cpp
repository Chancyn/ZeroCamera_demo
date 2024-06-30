// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"
#include "zc_msg_codec.h"
#include "zc_msg_rtsp.h"
#include "zc_msg_sys.h"
#include "zc_proc.h"
#include "zc_type.h"
#include <functional>

#include "ZcModCli.hpp"
#include "ZcType.hpp"

namespace zc {
static const char *g_Modurltab[ZC_MODID_BUTT] = {
    ZC_SYS_URL_IPC,
    ZC_CODEC_URL_IPC,
    ZC_RTSP_URL_IPC,
};

static const char *g_Modnametab[ZC_MODID_BUTT] = {
    ZC_SYS_MODNAME,
    ZC_CODEC_MODNAME,
    ZC_RTSP_MODNAME,
};

static inline const char *get_url_bymodid(ZC_U8 modid) {
    if (modid >= ZC_MODID_BUTT) {
        return nullptr;
    }
    return g_Modurltab[modid];
}

static inline const char *get_name_bymodid(ZC_U8 modid) {
    if (modid >= ZC_MODID_BUTT) {
        return nullptr;
    }
    return g_Modnametab[modid];
}

const char *CModCli::GetUrlbymodid(ZC_U8 modid) {
    return get_url_bymodid(modid);
}

CModCli::CModCli(ZC_U8 modid, ZC_U32 version) : m_modid(modid), m_seqno(0), m_version(version) {
    m_pid = getpid();

    strncpy(m_url, get_url_bymodid(m_modid), sizeof(m_url) - 1);
    strncpy(m_name, get_name_bymodid(m_modid), sizeof(m_name) - 1);
}

CModCli::~CModCli() {}

bool CModCli::BuildReqMsgHdr(zc_msg_t *pmsg, ZC_U8 modidto, ZC_U16 id, ZC_U16 sid, ZC_U8 chn, ZC_U32 size) {
    if (!pmsg) {
        pmsg = reinterpret_cast<zc_msg_t *>(new char[sizeof(zc_msg_t) + size]());
    }
    pmsg->pid = m_pid;
    pmsg->modid = m_modid;
    pmsg->ver = ZC_MSG_VERSION;
    pmsg->modidto = modidto;
    pmsg->msgtype = ZC_MSG_TYPE_REQ_E;
    pmsg->chn = chn;
    pmsg->id = id;
    pmsg->sid = sid;
    pmsg->size = size;
    pmsg->err = 0;
    pmsg->ts = zc_system_time();

    return true;
}

bool CModCli::MsgSendTo(zc_msg_t *pmsg, const char *urlto, zc_msg_t *prmsg, size_t *buflen) {
    pmsg->ts = zc_system_time();
    pmsg->seq = m_seqno++;
    CMsgCommReqClient cli;
    cli.Open(urlto);
    return cli.SendTo(pmsg, sizeof(zc_msg_t) + pmsg->size, prmsg, buflen);
}

bool CModCli::MsgSendTo(zc_msg_t *pmsg, zc_msg_t *prmsg, size_t *buflen) {
    pmsg->ts = zc_system_time();
    pmsg->seq = m_seqno++;
    CMsgCommReqClient cli;
    cli.Open(GetUrlbymodid(pmsg->modidto));
    return cli.SendTo(pmsg, sizeof(zc_msg_t) + pmsg->size, prmsg, buflen);
}

// send keepalive
int CModCli::_sendSMgrGetInfo(unsigned int type, unsigned int chn) {
    // LOG_TRACE("send register msg into[%s] into", m_name);
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_smgr_get_t)] = {0};
    zc_msg_t *req = reinterpret_cast<zc_msg_t *>(msg_buf);
    BuildReqMsgHdr(req, ZC_MODID_SYS_E, ZC_MID_SYS_SMGR_E, ZC_MSID_SMGR_GET_E, 0, sizeof(zc_mod_smgr_get_t));
    zc_mod_smgr_get_t *reqinfo = reinterpret_cast<zc_mod_smgr_get_t *>(req->data);
    reqinfo->type = type;
    reqinfo->chn = chn;

    // recv
    char rmsg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_smgr_get_rep_t)] = {0};
    zc_msg_t *rep = reinterpret_cast<zc_msg_t *>(rmsg_buf);
    size_t rlen = sizeof(zc_msg_t) + sizeof(zc_mod_smgr_get_rep_t);
    zc_mod_smgr_get_rep_t *repinfo = reinterpret_cast<zc_mod_smgr_get_rep_t *>(rep->data);
    if (MsgSendTo(req, ZC_SYS_URL_IPC, rep, &rlen)) {
        if (rep->err != 0) {
            LOG_ERROR("recv register rep err:%d \n", rep->err);
        }
    }

#if ZC_DEBUG
    // _dumpStreamInfo("recv streaminfo", &repinfo->info);
    uint64_t now = zc_system_time();
    LOG_TRACE("sendto smgrgetinfo : pid:%d,modid:%d,mod:%s, type:%u,chn:%u, cos1:%llu,%llu", req->pid, req->modid,
              m_name, reqinfo->type, reqinfo->chn, (rep->ts1 - rep->ts), (now - rep->ts));
#endif
    return 0;
}

}  // namespace zc
