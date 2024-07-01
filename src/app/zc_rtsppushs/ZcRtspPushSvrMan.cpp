// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "ZcModCli.hpp"
#include "ZcRtspPushServer.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"
#include "zc_stream_mgr.h"

#include "ZcRtspPushSvrMan.hpp"
#include "ZcType.hpp"

namespace zc {
// modsyscli
CRtspPushSvrMan::CRtspPushSvrMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0) {}

CRtspPushSvrMan::~CRtspPushSvrMan() {
    UnInit();
}

int CRtspPushSvrMan::_getStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli get info chn:%d", chn);
    // TODO(zhoucc): cb
    int ret = _sendSMgrGetInfo(ZC_SHMSTREAM_PUSHS, chn, info);
    if (ret < 0) {
        return ret;
    }
    memcpy(&m_mediainfo, info, sizeof(zc_stream_info_t));
    return ret;
}

int CRtspPushSvrMan::getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CRtspPushSvrMan *pRtsp = reinterpret_cast<CRtspPushSvrMan *>(ptr);
    return pRtsp->_getStreamInfoCb(chn, info);
}

int CRtspPushSvrMan::_setStreamInfoCb(unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("rtspcli set info type:%u, chn:%d", chn);
    // TODO(zhoucc): cb
    if (memcmp(&m_mediainfo, info, sizeof(zc_stream_info_t)) != 0) {
        return _sendSMgrGetInfo(ZC_SHMSTREAM_PUSHS, chn, info);
    }
    return 0;
}

int CRtspPushSvrMan::setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info) {
    CRtspPushSvrMan *pRtsp = reinterpret_cast<CRtspPushSvrMan *>(ptr);
    return pRtsp->_setStreamInfoCb(chn, info);
}

bool CRtspPushSvrMan::Init() {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    rtsppushs_callback_info_t cbinfo = {
        .GetInfoCb = getStreamInfoCb,
        .SetInfoCb = setStreamInfoCb,
        .MgrContext = this,
    };

    if (!CRtspPushServer::Init(&cbinfo)) {
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

bool CRtspPushSvrMan::_unInit() {
    Stop();
    CRtspPushServer::UnInit();

    return false;
}

bool CRtspPushSvrMan::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}
bool CRtspPushSvrMan::Start() {
    if (m_running) {
        return false;
    }
    CRtspPushServer::Start();
    m_running = true;

    return m_running;
}

bool CRtspPushSvrMan::Stop() {
    if (!m_running) {
        return false;
    }

    CRtspPushServer::Start();
    m_running = false;
    return true;
}

#if 1  // ZC_DEBUG_DUMP
static inline void _dumpTrackInfo(const char *user, const zc_meida_track_t *info) {
    LOG_TRACE("[%s] ch:%u,trackno:%u,track:%u,encode:%u,mediacode:%u,en:%u,size:%u,fmaxlen:%u, name:%s", user,
              info->chn, info->trackno, info->tracktype, info->encode, info->mediacode, info->enable, info->fifosize,
              info->framemaxlen, info->name);
    return;
}

static inline void _dumpStreamInfo(const char *user, const zc_stream_info_t *info) {
    LOG_TRACE("[%s] type:%d,idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype, info->idx, info->chn,
              info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}
#endif

// send get streaminfo
int CRtspPushSvrMan::_sendSMgrGetInfo(unsigned int type, unsigned int chn, zc_stream_info_t *info) {
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
    // _dumpStreamInfo("recv streaminfo", &repinfo->info);
    uint64_t now = zc_system_time();
    LOG_TRACE("smgr getinfo : pid:%d,modid:%d, type:%u,chn:%u, cos1:%llu,%llu", req->pid, req->modid, reqinfo->type,
              reqinfo->chn, (rep->ts1 - rep->ts), (now - rep->ts));
#endif
    return 0;
}

// send set streaminfo
int CRtspPushSvrMan::_sendSMgrSetInfo(unsigned int type, unsigned int chn, const zc_stream_info_t *info) {
    // LOG_TRACE("send register msg into[%s] into", m_name);
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_smgr_set_t)] = {0};
    zc_msg_t *req = reinterpret_cast<zc_msg_t *>(msg_buf);
    BuildReqMsgHdr(req, ZC_MODID_SYS_E, ZC_MID_SYS_SMGR_E, ZC_MSID_SMGR_SET_E, 0, sizeof(zc_mod_smgr_set_t));
    zc_mod_smgr_set_t *reqinfo = reinterpret_cast<zc_mod_smgr_set_t *>(req->data);
    reqinfo->type = type;
    reqinfo->chn = chn;
    memcpy(&reqinfo->info, info, sizeof(zc_stream_info_t));
    // recv
    char rmsg_buf[sizeof(zc_msg_t)] = {0};
    zc_msg_t *rep = reinterpret_cast<zc_msg_t *>(rmsg_buf);
    size_t rlen = sizeof(zc_msg_t) + sizeof(zc_mod_smgr_get_rep_t);
    if (MsgSendTo(req, ZC_SYS_URL_IPC, rep, &rlen)) {
        if (rep->err != 0) {
            LOG_ERROR("recv register rep err:%d \n", rep->err);
            return -1;
        }
    } else {
        // TODO(zhoucc):
        return -1;
    }

#if ZC_DEBUG
    _dumpStreamInfo("set streaminfo", info);
    uint64_t now = zc_system_time();
    LOG_TRACE("smgr setinfo : pid:%d,modid:%d, type:%u,chn:%u, cos1:%llu,%llu", req->pid, req->modid, reqinfo->type,
              reqinfo->chn, (rep->ts1 - rep->ts), (now - rep->ts));
#endif
    return 0;
}
}  // namespace zc
