// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcRtspPushCliMan.hpp"
#include "ZcType.hpp"

namespace zc {
// modsyscli
CRtspPushCliMan::CRtspPushCliMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0) {}

CRtspPushCliMan::~CRtspPushCliMan() {
    UnInit();
}

bool CRtspPushCliMan::Init(unsigned int type, unsigned int chn, const char *url, int transport) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }
    zc_stream_info_t info;

    if (_sendSMgrGetInfo(type, chn, &info) < 0) {
        LOG_TRACE("_sendSMgrGetInfo error");
        goto _err;
    }

    if (!CRtspPushClient::Init(info, url, transport)) {
        LOG_TRACE("CModRtsp Init error");
        goto _err;
    }

    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    _unInit();

    LOG_TRACE("Init error");
    return false;
}  // namespace zc

bool CRtspPushCliMan::_unInit() {
    Stop();
    CRtspPushClient::UnInit();

    return false;
}

bool CRtspPushCliMan::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}
bool CRtspPushCliMan::Start() {
    if (m_running) {
        return false;
    }

    m_running = CRtspPushClient::StartCli();
    return m_running;
}

bool CRtspPushCliMan::Stop() {
    if (!m_running) {
        return false;
    }

    CRtspPushClient::StopCli();
    m_running = false;
    return true;
}

#if 1  // ZC_DEBUG_DUMP
static inline void _dumpTrackInfo(const char *user, zc_meida_track_t *info) {
    LOG_TRACE("[%s] ch:%u,trackno:%u,track:%u,encode:%u,mediacode:%u,en:%u,size:%u,fmaxlen:%u, name:%s", user,
              info->chn, info->trackno, info->tracktype, info->encode, info->mediacode, info->enable, info->fifosize,
              info->framemaxlen, info->name);
    return;
}

static inline void _dumpStreamInfo(const char *user, zc_stream_info_t *info) {
    LOG_TRACE("[%s] type:%d,idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype, info->idx, info->chn,
              info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}
#endif

// send get streaminfo
int CRtspPushCliMan::_sendSMgrGetInfo(unsigned int type, unsigned int chn, zc_stream_info_t *info) {
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
            return -1;
        }
    } else {
        // TODO(zhoucc):
        return -1;
    }

    memcpy(info, &repinfo->info, sizeof(zc_stream_info_t));
#if ZC_DEBUG
    _dumpStreamInfo("recv streaminfo", info);
    uint64_t now = zc_system_time();
    LOG_TRACE("smgr getinfo : pid:%d,modid:%d, type:%u,chn:%u, cos1:%llu,%llu", req->pid, req->modid, reqinfo->type,
              reqinfo->chn, (rep->ts1 - rep->ts), (now - rep->ts));
#endif
    return 0;
}
}  // namespace zc
