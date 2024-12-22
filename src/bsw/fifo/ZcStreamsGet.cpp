// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <asm-generic/errno.h>
#include <cstdint>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <list>
#include <memory>
#include <mutex>

#include "zc_basic_fun.h"
#include "zc_h26x_sps_parse.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcStreamsGet.hpp"

#include "ZcType.hpp"


namespace zc {

typedef struct {
    CShmStreamR *fifor;
    unsigned int chn;
    unsigned int datalen;
    const uint8_t *dataptr;
} zc_write_frame_t;


CStreamsGet::CStreamsGet()
    : m_status(0), m_chn(0) {
    memset(&m_info, 0, sizeof(m_info));
}

CStreamsGet::~CStreamsGet() {
    UnInit();
}

bool CStreamsGet::Init(int chn) {
    LOG_TRACE("Init into");
    m_chn = chn;

    createStream();
    return true;
}

bool CStreamsGet::createStream() {
   uint32_t size = 0;
   zc_meida_track_t *track = nullptr;
    for (unsigned int i = 0; i < m_info.streaminfo.tracknum; i++) {
        track = &m_info.streaminfo.tracks[i];
        CShmStreamR *fiforeader = new CShmStreamR(track->fifosize, track->name, track->chn);
        if (!fiforeader) {
            LOG_ERROR("Create m_fiforeader error");
            continue;
        }

        if (!fiforeader->ShmAlloc()) {
            LOG_ERROR("ShmAlloc error");
            delete fiforeader;
            continue;
        }

        // skip2f
        // zc_frame_userinfo_t frameinfo;
        // Skip2LatestPos();
        if (fiforeader->GetStreamInfo(m_frameinfoTab[track->tracktype], true)) {
            LOG_WARN("Get Streaminfo/jump2latest ok");
        }
        m_fiforeader[i] = fiforeader;
        size++;
    }

    if (size <= 0) {
        LOG_ERROR("no stream error");
        return false;
    }

    LOG_TRACE("Init OK");
    return true;
}

bool CStreamsGet::destroyStream() {
    for (unsigned int i = 0; i < ZC_MEDIA_TRACK_BUTT; i++) {
        ZC_SAFE_DELETE(m_fiforeader[i]);
    }

    return true;
}

bool CStreamsGet::UnInit() {
    destroyStream();
    return true;
}

int CStreamsGet::_getFrameData(CShmStreamR *stream) {
    unsigned int ret = 0;
    zc_frame_t *pframe = (zc_frame_t *)m_framebuf;
    // (stream->Len() > sizeof(zc_frame_t)) {
    while (State() == Running && (stream->Len() > sizeof(zc_frame_t))) {
        ret = stream->Get(m_framebuf, sizeof(m_framebuf), sizeof(zc_frame_t));
        if (ret < sizeof(zc_frame_t)) {
            return -1;
        }

       // first IDR frame
        if (!m_Idr) {
            if (m_info.streaminfo.tracks[ZC_STREAM_VIDEO].enable && !pframe->keyflag) {
                LOG_WARN("drop, need IDR frame");
                return 0;
            } else {
                m_Idr = true;
                LOG_WARN("set IDR flag type:%d, video:%d, key:%d", pframe->type,
                         m_info.streaminfo.tracks[ZC_STREAM_VIDEO].enable, pframe->keyflag);
            }
        }

#if 1  // ZC_DEBUG
       // debug info
        if (pframe->keyflag) {
            uint64_t now = zc_system_time();
            LOG_TRACE("fmp4:pts:%u,utc:%u,now:%u,len:%d,cos:%dms", pframe->pts, pframe->utc, now, pframe->size,
                      now - pframe->utc);
        }
#endif
    }

    return 0;
}

int CStreamsGet::_packetProcess() {
    LOG_WARN("_packetProcess into");
    CEpoll ep{100};  // set timeout 100ms,for rtspsource thread exit
    int ret = 0;

    if (!ep.Create()) {
        LOG_ERROR("epoll create error");
        return -1;
    }

    for (unsigned int i = 0; i < ZC_MEDIA_TRACK_BUTT; i++) {
        int evfd = -1;
        if (m_fiforeader[i] && (evfd = m_fiforeader[i]->GetEvFd()) > 0) {
            LOG_WARN("epoll add fd[%d] ptr[%p]", evfd, m_fiforeader[i]);
            ep.Add(evfd, EPOLLIN | EPOLLET, m_fiforeader[i]);
        }
    }

    while (State() == Running) {
        ret = ep.Wait();
        if (ret == -1) {
            LOG_ERROR("epoll wait error");
            return -1;
        } else if (ret > 0) {
            for (int i = 0; i < ret; i++) {
                if (ep[i].events & EPOLLIN) {
                    CShmStreamR *stream = reinterpret_cast<CShmStreamR *>(ep[i].data.ptr);
                    // LOG_TRACE("epoll wait ok ret[%d], tack[%d]", ret, tack);
                    if (_getFrameData(stream) < 0) {
                        m_status = get_status_err;
                        LOG_ERROR("error _packetProcess exit");
                        return -1;
                    }
                }
            }
        }
    }

    LOG_WARN("_packetProcess exit");
    return 0;
}

int CStreamsGet::process() {
    LOG_WARN("process into");
    while (State() == Running) {
        if (_packetProcess() < 0) {
            break;
        }
        usleep(1000 * 1000);
    }
    LOG_WARN("process exit");
    return -1;
}
}  // namespace zc
