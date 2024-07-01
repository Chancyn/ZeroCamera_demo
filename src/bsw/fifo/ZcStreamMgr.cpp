// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// shm fifo

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mod/sys/zc_sys_smgr_handle.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcShmStream.hpp"
#include "ZcStreamMgr.hpp"
#include "ZcType.hpp"

#ifdef ZC_DEBUG
#define ZC_DEBUG_DUMP 1
#endif

namespace zc {

static const mgr_shmname_t g_nametab = {
    .tabs =
        {
            {
                "live",
                {ZC_STREAM_VIDEO_SHM_PATH, ZC_STREAM_AUDIO_SHM_PATH, ZC_STREAM_META_SHM_PATH},
            },

            {
                "pullc",
                {ZC_STREAM_VIDEOPULL_SHM_PATH, ZC_STREAM_AUDIOPULL_SHM_PATH, ZC_STREAM_METAPULL_SHM_PATH},
            },

            {
                "pushs",
                {ZC_STREAM_VIDEOPUSH_SHM_PATH, ZC_STREAM_AUDIOPUSH_SHM_PATH, ZC_STREAM_METAPUSH_SHM_PATH},
            },

            // pushc push live stream
            {
                "pushc",
                {ZC_STREAM_VIDEO_SHM_PATH, ZC_STREAM_AUDIO_SHM_PATH, ZC_STREAM_META_SHM_PATH},
            },
        },
};

// debug dump
#if ZC_DEBUG_DUMP
static inline void _dumpTrackInfo(const char *user, zc_meida_track_t *info) {
    LOG_TRACE("[%s] ch:%u,trackno:%u,track:%u,encode:%u,en:%u,size:%u,fmaxlen:%u, status:%u,name:%s", user, info->chn,
              info->trackno, info->tracktype, info->encode, info->enable, info->fifosize, info->framemaxlen,
              info->status, info->name);
    return;
}

static inline void _dumpStreamInfo(const char *user, zc_stream_info_t *info) {
    LOG_TRACE("[%s] type:%d(%s),idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype,
              g_nametab.tabs[info->shmstreamtype].name, info->idx, info->chn, info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}
#endif

static inline int transEncode2MediaCode(unsigned int encode) {
    int mediacode = -1;
    if (encode == ZC_FRAME_ENC_H264) {
        mediacode = ZC_MEDIA_CODE_H264;
    } else if (encode == ZC_FRAME_ENC_H265) {
        mediacode = ZC_MEDIA_CODE_H265;
    } else if (encode == ZC_FRAME_ENC_AAC) {
        mediacode = ZC_MEDIA_CODE_AAC;
    } else if (encode == ZC_FRAME_ENC_META_BIN) {
        mediacode = ZC_MEDIA_CODE_METADATA;
    }

    return mediacode;
}

static inline void _initTrackInfo(zc_meida_track_t *info, unsigned char chn, unsigned int trackno, unsigned char track,
                                  unsigned char encode, unsigned char enable, unsigned int fifosize,
                                  unsigned int framemaxlen, const char *name) {
    info->chn = chn;
    info->trackno = trackno;
    info->tracktype = track;
    info->encode = encode;
    info->mediacode = transEncode2MediaCode(encode);
    info->enable = enable;
    info->fifosize = fifosize;
    info->framemaxlen = framemaxlen;
    info->status = ZC_TRACK_STATUS_IDE;
    strncpy(info->name, name, sizeof(info->name));

    return;
}

CStreamMgr::CStreamMgr() : m_init(false), m_running(0) {}

CStreamMgr::~CStreamMgr() {
    UnInit();
}

void CStreamMgr::_initTracksInfo(zc_meida_track_t *info, unsigned char type, unsigned char chn, unsigned char venc,
                                 unsigned char aenc, unsigned char menc) {
    unsigned int trackno = 0;
    _initTrackInfo(info + ZC_STREAM_VIDEO, chn, trackno++, ZC_STREAM_VIDEO, venc, 1, ZC_STREAM_MAIN_VIDEO_SIZE,
                   ZC_STREAM_MAXFRAME_SIZE, m_nametab.tabs[type].tracksname[ZC_STREAM_VIDEO]);

    _initTrackInfo(info + ZC_STREAM_AUDIO, chn, trackno++, ZC_STREAM_AUDIO, aenc, 1, ZC_STREAM_AUDIO_SIZE,
                   ZC_STREAM_MAXFRAME_SIZE_A, m_nametab.tabs[type].tracksname[ZC_STREAM_AUDIO]);
    // meta disable
    _initTrackInfo(info + ZC_STREAM_META, chn, trackno++, ZC_STREAM_META, menc, 0, ZC_STREAM_META_SIZE,
                   ZC_STREAM_MAXFRAME_SIZE_M, m_nametab.tabs[type].tracksname[ZC_STREAM_META]);

    return;
}

bool CStreamMgr::Init(zc_stream_mgr_cfg_t *cfg) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    unsigned char total = 0;
    unsigned char totalpushc = 0;
    unsigned int idx = 0;

    // init path
    memcpy(&m_nametab, &g_nametab, sizeof(mgr_shmname_t));
    zc_stream_info_t *ptab = nullptr;
    zc_stream_info_t *info = nullptr;
    zc_stream_mgr_cfg_t tmp = {
        {
            ZC_STREAMMGR_LIVE_MAX_CHN,
            ZC_STREAMMGR_PULLC_MAX_CHN,
            ZC_STREAMMGR_PUSHS_MAX_CHN,
            ZC_STREAMMGR_PUSHC_MAX_CHN,
        },
        ZC_STREAMMGR_DECODE_MAX_CHN,
    };

    if (cfg) {
        memcpy(&tmp, cfg, sizeof(tmp));
    }
    for (unsigned int type = 0; type < ZC_SHMSTREAM_PUSHS; type++) {
        totalpushc += tmp.maxchn[type];
    }

    // limit push client maxchn
    tmp.maxchn[ZC_SHMSTREAM_PUSHC] =
        tmp.maxchn[ZC_SHMSTREAM_PUSHC] < totalpushc ? tmp.maxchn[ZC_SHMSTREAM_PUSHC] : totalpushc;

    for (unsigned int type = 0; type < ZC_SHMSTREAM_BUTT; type++) {
        total += tmp.maxchn[type];
    }

    zc_frame_enc_e venc = ZC_FRAME_ENC_H264;
    zc_frame_enc_e aenc = ZC_FRAME_ENC_AAC;
    zc_frame_enc_e menc = ZC_FRAME_ENC_META_BIN;

    ptab = new zc_stream_info_t[total]();
    if (!ptab) {
        LOG_TRACE("new streaminfo error");
        goto _err;
    }
    info = ptab;
    for (unsigned int type = 0; type < ZC_SHMSTREAM_BUTT; type++) {
        for (unsigned int chn = 0; chn < tmp.maxchn[type]; chn++) {
            info->chn = chn;
            info->idx = idx;
            info->status = ZC_STREAM_STATUS_INIT;
            info->tracknum = ZC_STREAMMGR_TRACK_MAX_NUM;
            info->shmstreamtype = type;

            // TODO(zhoucc): chn0 H265/ ch1 H264
            venc = chn == 0 ? ZC_FRAME_ENC_H265 : ZC_FRAME_ENC_H264;
            _initTracksInfo(&info->tracks[0], type, chn, venc, aenc, menc);
#if ZC_DEBUG_DUMP
            _dumpStreamInfo("init", info);
#endif
            info++;
            idx++;
        }
    }

    m_total = total;
    m_infoTab = ptab;
    memcpy(&m_cfg, &tmp, sizeof(tmp));
    m_init = true;
    for (unsigned int type = 0; type < ZC_SHMSTREAM_PUSHS; type++) {
        LOG_TRACE("Init ok idx:%d, num:%d ", tmp.maxchn[type]);
    }
    LOG_TRACE("init streaminfo ok, m_total:%u", m_total);
    return true;

_err:
    _unInit();

    LOG_TRACE("Init error");
    return false;
}
bool CStreamMgr::_unInit() {
    Stop();
    return false;
}

bool CStreamMgr::UnInit() {
    if (!m_init) {
        return true;
    }

    _unInit();

    m_init = false;
    return false;
}

int CStreamMgr::_findIdx(zc_shmstream_e type, unsigned int nchn) {
    if (type < 0 || type >= ZC_SHMSTREAM_BUTT) {
        LOG_TRACE("error type:%d", type);
        return -1;
    }

    if (nchn >= m_cfg.maxchn[type]) {
        LOG_TRACE("error nchn:%d", nchn);
        return -1;
    }

    int idx = 0;
    for (unsigned int i = 0; i < type; i++) {
        idx += m_cfg.maxchn[i];
    }
    idx += nchn;
    return idx;
}

int CStreamMgr::getCount(unsigned int type) {
    if (type == ZC_SHMSTREAM_ALL) {
        return m_total;
    } else if (type >= 0 || type < ZC_SHMSTREAM_BUTT) {
        return m_cfg.maxchn[type];
    }

    return 0;
}

inline int CStreamMgr::_getShmStreamInfo(zc_stream_info_t *info, int idx, unsigned int count) {
    ZC_ASSERT(count > 0);
    ZC_ASSERT(idx + count <= m_total);
    memcpy(info, &m_infoTab[idx], sizeof(zc_stream_info_t) * count);

    return 0;
}

int CStreamMgr::getALLShmStreamInfo(zc_stream_info_t *info, unsigned int type, unsigned int count) {
    int tmpc = getCount(type);
    count = count <= tmpc ? count : tmpc;
    if (count > 0) {
        int idx = 0;
        if (type != ZC_SHMSTREAM_ALL)
            _findIdx((zc_shmstream_e)type, 0);

        std::lock_guard<std::mutex> locker(m_mutex);
        _getShmStreamInfo(info, idx, count);
    }

    return 0;
}

int CStreamMgr::getShmStreamInfo(zc_stream_info_t *info, unsigned int type, unsigned int nchn) {
    int idx = _findIdx((zc_shmstream_e)type, nchn);
    if (idx < 0) {
        return -1;
    }
    std::lock_guard<std::mutex> locker(m_mutex);

    return _getShmStreamInfo(info, idx, 1);
}

int CStreamMgr::_setShmStreamInfo(zc_stream_info_t *info, int idx) {
    ZC_ASSERT(idx < m_total);
    // TODO(zhoucc): check param
    memcpy(&m_infoTab[idx], info, sizeof(zc_stream_info_t));

    return 0;
}

int CStreamMgr::setShmStreamInfo(zc_stream_info_t *info, unsigned int type, unsigned int nchn) {
    int idx = _findIdx((zc_shmstream_e)type, nchn);
    if (idx < 0) {
        return -1;
    }
    std::lock_guard<std::mutex> locker(m_mutex);

    return _setShmStreamInfo(info, idx);
}
#if 0
int CStreamMgr::_createStreamW(int idx) {
    ZC_ASSERT(idx < m_total);
    zc_stream_info_t tmp;
    zc_stream_info_t *info = &m_infoTab[idx];
    memcpy(&tmp, info, sizeof(zc_stream_info_t));
    if (tmp.status < ZC_TRACK_STATUS_W) {
        tmp.status = ZC_TRACK_STATUS_W;
    }

    ZC_ASSERT(idx == tmp.idx);
    tmp.tracknum = 2;
    memcpy(info, &tmp, sizeof(zc_stream_info_t));

#if ZC_DEBUG_DUMP
    _dumpStreamInfo("createw", info);
#endif

    return idx;
_err:
    LOG_ERROR("createw error, idx[%d]", idx);
    return -1;
}

int CStreamMgr::CreateStreamW(zc_shmstream_e type, unsigned int nchn) {
    if (!m_running) {
        return -1;
    }

    int idx = _findIdx(type, nchn);
    if (idx < 0) {
        return -1;
    }

    std::lock_guard<std::mutex> locker(m_mutex);

    return _createStreamW(idx);
}

int CStreamMgr::_createStreamR(int idx) {
    ZC_ASSERT(idx < m_total);
    zc_stream_info_t tmp;
    zc_stream_info_t *info = &m_infoTab[idx];
    memcpy(&tmp, info, sizeof(zc_stream_info_t));
    // not open write
    if (tmp.status < ZC_TRACK_STATUS_W) {
        LOG_ERROR("creater error, status:%d, idx:%d", tmp.status, idx);
        goto _err;
    }

    tmp.status = ZC_TRACK_STATUS_RW;
    ZC_ASSERT(idx == tmp.idx);
    tmp.tracknum = 2;
    memcpy(info, &tmp, sizeof(zc_stream_info_t));

#if ZC_DEBUG_DUMP
    _dumpStreamInfo("createw", info);
#endif
    LOG_TRACE("creater ok, idx[%d]", idx);
    return idx;
_err:
    LOG_TRACE("creater error, idx[%d]", idx);
    return -1;
}

int CStreamMgr::CreateStreamR(zc_shmstream_e type, unsigned int nchn) {
    if (!m_running) {
        return -1;
    }

    int idx = _findIdx(type, nchn);
    if (idx < 0) {
        return -1;
    }

    std::lock_guard<std::mutex> locker(m_mutex);

    return _createStreamR(idx);
}

int CStreamMgr::DestoryStream(int idx) {
    if (!m_running) {
        return -1;
    }

    if (idx < 0 || idx >= m_total) {
        return -1;
    }

    std::lock_guard<std::mutex> locker(m_mutex);

    return _destoryStream(idx);
}
#endif

bool CStreamMgr::Start() {
    if (m_running) {
        return false;
    }

    Thread::Start();
    m_running = true;
    return true;
}

bool CStreamMgr::Stop() {
    if (!m_running) {
        return false;
    }

    Thread::Stop();
    m_running = false;
    return true;
}

int CStreamMgr::process() {
    LOG_WARN("process into\n");
    int ret = 0;

    while (State() == Running) {
        // TODO(zhoucc): do something
        usleep(10 * 1000);
    }
_err:
    LOG_WARN("process exit\n");
    return ret;
}

int CStreamMgr::HandleCtrl(unsigned int type, void *indata, void *outdata) {
    LOG_WARN("HandleCtrl %u", type);
    if (!m_running) {
        return -1;
    }
    int ret = 0;
    switch (type) {
    case SYS_SMGR_HDL_REGISTER_E:
        break;
    case SYS_SMGR_HDL_UNREGISTER_E:
        break;
    case SYS_SMGR_HDL_GECOUNT_E: {
        zc_sys_smgr_getcount_in_t *in = reinterpret_cast<zc_sys_smgr_getcount_in_t *>(indata);
        zc_sys_smgr_getcount_out_t *out = reinterpret_cast<zc_sys_smgr_getcount_out_t *>(outdata);
        out->count = getCount(in->type);
        break;
    }
    case SYS_SMGR_HDL_GETALLINFO_E: {
        zc_sys_smgr_getallinfo_in_t *in = reinterpret_cast<zc_sys_smgr_getallinfo_in_t *>(indata);
        zc_sys_smgr_getallinfo_out_t *out = reinterpret_cast<zc_sys_smgr_getallinfo_out_t *>(outdata);
        ret = getALLShmStreamInfo(out->pinfo, in->type, in->count);
        break;
    }
    case SYS_SMGR_HDL_GETINFO_E: {
        zc_sys_smgr_getinfo_in_t *in = reinterpret_cast<zc_sys_smgr_getinfo_in_t *>(indata);
        zc_sys_smgr_getinfo_out_t *out = reinterpret_cast<zc_sys_smgr_getinfo_out_t *>(outdata);
        ret = getShmStreamInfo(&out->info, in->type, in->chn);
        _dumpStreamInfo("getstream", &out->info);
        LOG_WARN("SYS_SMGR_HDL_GETINFO_E type:%u, chn:%u, ret:%u", in->type, in->chn, ret);
        break;
    }
    case SYS_SMGR_HDL_SETINFO_E: {
        zc_sys_smgr_setinfo_in_t *in = reinterpret_cast<zc_sys_smgr_setinfo_in_t *>(indata);
        zc_sys_smgr_setinfo_out_t *out = reinterpret_cast<zc_sys_smgr_setinfo_out_t *>(outdata);
        // copy to out;set and update out info
        memcpy(&out->info, &in->info, sizeof(zc_stream_info_t));
        ret = setShmStreamInfo(&out->info, in->type, in->chn);
        break;
    }
    default:
        ret = -1;
        break;
    }

    return ret;
}

}  // namespace zc
