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

#include "Thread.hpp"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

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
static inline void _dumpTrackInfo(const char *user, zc_shmstream_track_t *info) {
    LOG_TRACE("[%s] ch:%u,track:%u,encode:%u,en:%u,size:%u,status:%u,name:%s", user, info->chn, info->tracktype,
              info->encode, info->enable, info->fifosize, info->status, info->name);
    return;
}

static inline void _dumpStreamInfo(const char *user, zc_shmstream_info_t *info) {
    LOG_TRACE("[%s] type:%d(%s),idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype,
              g_nametab.tabs[info->shmstreamtype].name, info->idx, info->chn, info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}
#endif

static inline void _initTrackInfo(zc_shmstream_track_t *info, unsigned char chn, unsigned char track,
                                  unsigned char encode, unsigned char enable, unsigned int fifosize, const char *name) {
    info->chn = chn;
    info->tracktype = track;
    info->encode = encode;
    info->enable = enable;
    info->fifosize = fifosize;
    info->status = ZC_TRACK_STATUS_IDE;
    strncpy(info->name, name, sizeof(info->name));

    return;
}

CStreamMgr::CStreamMgr() : m_init(false), m_running(0) {}

CStreamMgr::~CStreamMgr() {
    UnInit();
}

void CStreamMgr::_initTracksInfo(zc_shmstream_track_t *info, unsigned char type, unsigned char chn, unsigned char venc,
                                 unsigned char aenc, unsigned char menc) {
    _initTrackInfo(info + ZC_STREAM_VIDEO, chn, ZC_STREAM_VIDEO, venc, 1, ZC_STREAM_MAXFRAME_SIZE,
                   m_nametab.tabs[type].tracksname[ZC_STREAM_VIDEO]);

    _initTrackInfo(info + ZC_STREAM_AUDIO, chn, ZC_STREAM_AUDIO, aenc, 1, ZC_STREAM_MAXFRAME_SIZE_A,
                   m_nametab.tabs[type].tracksname[ZC_STREAM_AUDIO]);
    // meta disable
    _initTrackInfo(info + ZC_STREAM_META, chn, ZC_STREAM_META, menc, 0, ZC_STREAM_MAXFRAME_SIZE_M,
                   m_nametab.tabs[type].tracksname[ZC_STREAM_META]);

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
    zc_shmstream_info_t *ptab = nullptr;
    zc_shmstream_info_t *info = nullptr;
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
    for (unsigned int type = 0; type < ZC_SHMSTREAM_TYPE_PUSHS; type++) {
        totalpushc += tmp.maxchn[type];
    }

    // limit push client maxchn
    tmp.maxchn[ZC_SHMSTREAM_TYPE_PUSHC] =
        tmp.maxchn[ZC_SHMSTREAM_TYPE_PUSHC] < totalpushc ? tmp.maxchn[ZC_SHMSTREAM_TYPE_PUSHC] : totalpushc;

    for (unsigned int type = 0; type < ZC_SHMSTREAM_TYPE_PUSHS; type++) {
        total += tmp.maxchn[type];
    }

    zc_frame_enc_e venc = ZC_FRAME_ENC_H264;
    zc_frame_enc_e aenc = ZC_FRAME_ENC_AAC;
    zc_frame_enc_e menc = ZC_FRAME_ENC_META_BIN;

    ptab = new zc_shmstream_info_t[total]();
    if (!ptab) {
        LOG_TRACE("new streaminfo error");
        goto _err;
    }
    info = ptab;
    for (unsigned int type = 0; type < ZC_SHMSTREAM_TYPE_BUTT; type++) {
        for (unsigned int chn = 0; chn < tmp.maxchn[type]; chn++) {
            info->chn = chn;
            info->idx = idx;
            info->status = ZC_STREAM_STATUS_INIT;
            info->tracknum = 0;
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
    for (unsigned int type = 0; type < ZC_SHMSTREAM_TYPE_PUSHS; type++) {
        LOG_TRACE("Init ok idx:%d, num:%d ", tmp.maxchn[type]);
    }

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

int CStreamMgr::_findIdx(zc_shmstream_type_e type, unsigned int nchn) {
    if (type < 0 || type >= ZC_SHMSTREAM_TYPE_BUTT) {
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

int CStreamMgr::_getShmStreamInfo(zc_shmstream_info_t *info, int idx) {
    ZC_ASSERT(idx < m_total);
    memcpy(info, &m_infoTab[idx], sizeof(zc_shmstream_info_t));

    return 0;
}

int CStreamMgr::GetShmStreamInfo(zc_shmstream_info_t *info, zc_shmstream_type_e type, unsigned int nchn) {
    if (m_running) {
        return -1;
    }

    if (m_running) {
        return -1;
    }

    int idx = _findIdx(type, nchn);
    if (idx < 0) {
        return -1;
    }

    std::lock_guard<std::mutex> locker(m_mutex);

    return _getShmStreamInfo(info, idx);
}

#if 0
int CStreamMgr::_createStreamW(int idx) {
    ZC_ASSERT(idx < m_total);
    zc_shmstream_info_t tmp;
    zc_shmstream_info_t *info = &m_infoTab[idx];
    memcpy(&tmp, info, sizeof(zc_shmstream_info_t));
    if (tmp.status < ZC_TRACK_STATUS_W) {
        tmp.status = ZC_TRACK_STATUS_W;
    }

    ZC_ASSERT(idx == tmp.idx);
    tmp.tracknum = 2;
    memcpy(info, &tmp, sizeof(zc_shmstream_info_t));

#if ZC_DEBUG_DUMP
    _dumpStreamInfo("createw", info);
#endif

    return idx;
_err:
    LOG_ERROR("createw error, idx[%d]", idx);
    return -1;
}

int CStreamMgr::CreateStreamW(zc_shmstream_type_e type, unsigned int nchn) {
    if (m_running) {
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
    zc_shmstream_info_t tmp;
    zc_shmstream_info_t *info = &m_infoTab[idx];
    memcpy(&tmp, info, sizeof(zc_shmstream_info_t));
    // not open write
    if (tmp.status < ZC_TRACK_STATUS_W) {
        LOG_ERROR("creater error, status:%d, idx:%d", tmp.status, idx);
        goto _err;
    }

    tmp.status = ZC_TRACK_STATUS_RW;
    ZC_ASSERT(idx == tmp.idx);
    tmp.tracknum = 2;
    memcpy(info, &tmp, sizeof(zc_shmstream_info_t));

#if ZC_DEBUG_DUMP
    _dumpStreamInfo("createw", info);
#endif
    LOG_TRACE("creater ok, idx[%d]", idx);
    return idx;
_err:
    LOG_TRACE("creater error, idx[%d]", idx);
    return -1;
}

int CStreamMgr::CreateStreamR(zc_shmstream_type_e type, unsigned int nchn) {
    if (m_running) {
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
    if (m_running) {
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
    int64_t dts = 0;

    while (State() == Running) {
        // TODO(zhoucc): do something
        usleep(10 * 1000);
    }
_err:
    LOG_WARN("process exit\n");
    return ret;
}
}  // namespace zc
