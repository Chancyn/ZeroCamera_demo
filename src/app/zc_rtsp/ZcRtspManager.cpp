// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcRtspManager.hpp"
#include "ZcRtspServer.hpp"
#include "ZcType.hpp"

namespace zc {
CRtspManager::CRtspManager() : m_init(false), m_running(0) {}

CRtspManager::~CRtspManager() {
    UnInit();
}

int CRtspManager::_getStreamInfoCb(unsigned int type, unsigned int chn, zc_media_info_t *info) {
    LOG_TRACE("get info type:%u, chn:%d", type, chn);
    // TODO(zhoucc): cb
    return _sendSMgrGetInfo(type, chn, info);
}

int CRtspManager::getStreamInfoCb(void *ptr, unsigned int type, unsigned int chn, zc_media_info_t *info) {
    CRtspManager *pRtsp = reinterpret_cast<CRtspManager *>(ptr);
    return pRtsp->_getStreamInfoCb(type, chn, info);
}

bool CRtspManager::Init(rtsp_callback_info_t *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }
    rtspsvr_cb_info_t stSvrCbinfo = {
        .getStreamInfoCb = CRtspManager::getStreamInfoCb,
        .MgrContext = this,
    };

    if (!CModRtsp::Init(cbinfo)) {
        LOG_TRACE("CModRtsp Init error");
        goto _err;
    }

    if (!CRtspServer::Init(&stSvrCbinfo)) {
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

bool CRtspManager::_unInit() {
    Stop();
    CModRtsp::UnInit();
    CRtspServer::UnInit();
    return false;
}

bool CRtspManager::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}
bool CRtspManager::Start() {
    if (m_running) {
        return false;
    }

    CModRtsp::Start();
    CRtspServer::Start();
    m_running = true;
    return true;
}

bool CRtspManager::Stop() {
    if (!m_running) {
        return false;
    }

    CModRtsp::Stop();
    CRtspServer::Stop();
    m_running = false;
    return true;
}

#if 1  // ZC_DEBUG_DUMP
static inline void _dumpTrackInfo(const char *user, zc_meida_track_t *info) {
    LOG_TRACE("[%s] ch:%u,trackno:%u,track:%u,encode:%u,mediacode:%u,en:%u,size:%u,fmaxlen:%u,name:%s", user, info->chn,
              info->trackno, info->tracktype, info->encode, info->mediacode, info->enable, info->fifosize,
              info->framemaxlen, info->name);
    return;
}

static inline void _dumpStreamInfo(const char *user, zc_media_info_t *info) {
    LOG_TRACE("[%s] type:%d,idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype, info->idx, info->chn,
              info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}
#endif

static inline void _mediainfo_trans(zc_media_info_t *info, const zc_mod_smgr_iteminfo_t *modinfo) {
    info->shmstreamtype = modinfo->shmstreamtype;
    info->chn = modinfo->chn;
    info->idx = modinfo->idx;
    info->tracknum = modinfo->tracknum;
    info->status = modinfo->status;

    for (unsigned int i = 0; i < modinfo->tracknum && i < ZC_MSG_TRACK_MAX_NUM; i++) {
        info->tracks[i].chn = modinfo->tracks[i].chn;
        info->tracks[i].trackno = modinfo->tracks[i].trackno;
        info->tracks[i].tracktype = modinfo->tracks[i].tracktype;
        info->tracks[i].encode = modinfo->tracks[i].encode;
        if (modinfo->tracks[i].encode == ZC_FRAME_ENC_H264) {
            info->tracks[i].mediacode = ZC_MEDIA_CODE_H264;
        } else if (modinfo->tracks[i].encode == ZC_FRAME_ENC_H265) {
            info->tracks[i].mediacode = ZC_MEDIA_CODE_H265;
        } else if (modinfo->tracks[i].encode == ZC_FRAME_ENC_AAC) {
            info->tracks[i].mediacode = ZC_MEDIA_CODE_AAC;
        } else if (modinfo->tracks[i].encode == ZC_FRAME_ENC_META_BIN) {
            info->tracks[i].mediacode = ZC_MEDIA_CODE_METADATA;
        }
        info->tracks[i].enable = modinfo->tracks[i].enable;
        info->tracks[i].fifosize = modinfo->tracks[i].fifosize;
        info->tracks[i].framemaxlen = modinfo->tracks[i].framemaxlen;
        strncpy(info->tracks[i].name, modinfo->tracks[i].name, sizeof(info->tracks[i].name) - 1);
    }
    _dumpStreamInfo("user", info);
    return;
}

// send get streaminfo
int CRtspManager::_sendSMgrGetInfo(unsigned int type, unsigned int chn, zc_media_info_t *info) {
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

    _mediainfo_trans(info, &repinfo->info);
#if ZC_DEBUG
    // _dumpStreamInfo("recv streaminfo", &repinfo->info);
    uint64_t now = zc_system_time();
    LOG_TRACE("smgr getinfo : pid:%d,modid:%d, type:%u,chn:%u, cos1:%llu,%llu", req->pid, req->modid, reqinfo->type,
              reqinfo->chn, (rep->ts1 - rep->ts), (now - rep->ts));
#endif
    return 0;
}
}  // namespace zc
