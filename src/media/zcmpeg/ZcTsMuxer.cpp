// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "mpeg-ps.h"
#include "mpeg-ts.h"
#include "mpeg-util.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_proc.h"

#include "Epoll.hpp"
#include "Thread.hpp"
#include "zc_macros.h"
#include "ZcStreamTrace.hpp"
#include "ZcTsMuxer.hpp"
#include "ZcType.hpp"
#include "zc_type.h"

namespace zc {
enum zc_stream_id {
    ZC_PSI_STREAM_RESERVED = 0,
    ZC_PSI_STREAM_H264 = 0x1b,
    ZC_PSI_STREAM_H265 = 0x24,
    ZC_PSI_STREAM_AAC = 0xf,
};

const static int g_codeidTab[ZC_FRAME_ENC_BUTT] = {
    ZC_PSI_STREAM_H264,
    ZC_PSI_STREAM_H265,
    ZC_PSI_STREAM_AAC,
    ZC_PSI_STREAM_RESERVED,
};

CTsMuxer::CTsMuxer() : Thread("tsmuxer"), m_Idr(false), m_status(ts_status_init), m_ts(nullptr) {
    memset(&m_info, 0, sizeof(m_info));
    m_vector.clear();
    for (unsigned int i = 0; i < _SIZEOFTAB(m_streamid); i++) {
        m_streamid[i] = -1;
    }
    memset(m_pts, 0, sizeof(m_pts));
}

CTsMuxer::~CTsMuxer() {
    Destroy();
}

bool CTsMuxer::destroyStream() {
    auto iter = m_vector.begin();
    for (; iter != m_vector.end();) {
        ZC_SAFE_DELETE(*iter);
        iter = m_vector.erase(iter);
    }
    m_vector.clear();

    return true;
}

bool CTsMuxer::createStream() {
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

        if (fiforeader->GetStreamInfo(m_frameinfoTab[track->tracktype], true)) {
            LOG_WARN("Get Streaminfo/jump2latest ok");
        }
        m_vector.push_back(fiforeader);
    }

    if (m_vector.size() <= 0) {
        LOG_ERROR("no stream error");
        return false;
    }

    // check auido info
    zc_audio_naluinfo_t *ainfo = &m_frameinfoTab[ZC_MEDIA_TRACK_AUDIO].ainfo;
    ainfo->channels = ainfo->channels ? ainfo->channels : ZC_AUDIO_CHN;
    ainfo->sample_bits = ainfo->sample_bits ? ainfo->sample_bits : ZC_AUDIO_SAMPLE_BIT_16;
    ainfo->sample_rate = ainfo->sample_rate ? ainfo->sample_rate : ZC_AUDIO_FREQUENCE;

    return true;
}

void *CTsMuxer::tsAlloc(void *ptr, uint64_t bytes) {
    CTsMuxer *self = reinterpret_cast<CTsMuxer *>(ptr);
    return self->_tsAlloc(bytes);
}

void *CTsMuxer::_tsAlloc(uint64_t bytes) {
    ZC_ASSERT(bytes == ZC_N_TS_PACKET);
    return m_pkgbuf;
}

int CTsMuxer::tsWrite(void *ptr, const void *packet, size_t bytes) {
    CTsMuxer *self = reinterpret_cast<CTsMuxer *>(ptr);
    return self->_tsWrite(packet, bytes);
}

int CTsMuxer::_tsWrite(const void *packet, size_t bytes) {
    ZC_ASSERT(bytes == ZC_N_TS_PACKET);
    if (m_info.onTsPacketCb) {
        m_info.onTsPacketCb(m_info.Context, packet, bytes);
    }

    return 0;
}

void CTsMuxer::tsFree(void *ptr, void *packet) {
    CTsMuxer *self = reinterpret_cast<CTsMuxer *>(ptr);
    return self->_tsFree(packet);
}

void CTsMuxer::_tsFree(void *packet) {
    return;
}

bool CTsMuxer::Create(const zc_tsmuxer_info_t &info) {
    if (m_ts) {
        return false;
    }

    struct mpeg_ts_func_t tshandler;
    tshandler.alloc = tsAlloc;
    tshandler.write = tsWrite;
    tshandler.free = tsFree;

    memcpy(&m_info, &info, sizeof(m_info));
    if (!createStream()) {
        LOG_ERROR("create error");
        goto _err;
    }

    m_ts = mpeg_ts_create(&tshandler, this);
    if (!m_ts) {
        LOG_ERROR("create error");
        goto _err;
    }

    LOG_TRACE("create ok");
    return true;
_err:
    destroyStream();
    m_ts = nullptr;
    LOG_ERROR("create error");
    return false;
}

bool CTsMuxer::Destroy() {
    if (!m_ts) {
        return false;
    }

    Stop();
    mpeg_ts_destroy(m_ts);
    m_ts = nullptr;
    destroyStream();
    m_ts = nullptr;
    LOG_TRACE("destroy ok");
    return true;
}

bool CTsMuxer::Start() {
    if (!m_ts) {
        return false;
    }

    Thread::Start();
    m_status = ts_status_init;
    return true;
}

bool CTsMuxer::Stop() {
    Thread::Stop();
    return false;
}

enum pts_status_e {
    pts_leader_drop = -3,     // leader error
    pts_delay_drop = -2,      // delay error
    pts_waitvideo_drop = -1,  // error
    pts_check_ok = 0,         //
    pts_delay_warn,           //
    pts_leader_warn,          //
};

// pts_status_e < 0 drop, -1 audio delay over limit drop, -2 early leader over limit
int CTsMuxer::_checkPts(zc_frame_t *frame) {
    if (m_info.streaminfo.tracks[ZC_STREAM_VIDEO].enable) {
        if (m_pts[ZC_STREAM_VIDEO] == 0 && frame->type != ZC_STREAM_VIDEO) {
            LOG_WARN("wait video, drop type:%d,encode:%d,size:%u,vpts%llu,vpts%llu", frame->type, frame->video.encode,
                     frame->size, frame->pts, m_pts[ZC_STREAM_VIDEO]);
            return pts_waitvideo_drop;
        }

        if (frame->type == ZC_STREAM_AUDIO) {
            int diff = frame->pts - m_pts[ZC_STREAM_VIDEO];
            // delay
            if (diff > 0) {
                if (diff > ZC_AUIDO_DELAY_VIDEO) {
                    LOG_WARN("drop apts:%llu,delay pts:%llu,diff:%d,over%dms", frame->pts, m_pts, diff,
                             ZC_AUIDO_DELAY_VIDEO);
                    return pts_delay_drop;
                } else if (diff > ZC_AUIDO_DELAY_VIDEO_WARN) {
                    LOG_TRACE("warn apts:%llu,delay pts:%llu,diff:%,over%dms", frame->pts, m_pts, diff,
                              ZC_AUIDO_DELAY_VIDEO_WARN);
                    return pts_delay_warn;
                }
            } else {
                if (diff + ZC_AUIDO_LEADER_VIDEO < 0) {
                    LOG_WARN("drop apts:%llu,leader pts:%llu,diff:%d,over%dms", frame->pts, m_pts, (-1) * diff,
                             ZC_AUIDO_LEADER_VIDEO);
                    return pts_delay_drop;
                } else if (diff + ZC_AUIDO_LEADER_VIDEO_WARN < 0) {
                    LOG_TRACE("warn apts:%llu,leader pts:%llu,diff:%d,over%dms", frame->pts, m_pts, (-1) * diff,
                              ZC_AUIDO_LEADER_VIDEO_WARN);
                    return pts_delay_warn;
                }
            }
        }
    }

    return pts_check_ok;
}

int CTsMuxer::_tsAddStream(zc_frame_t *frame) {
    int avtype = 0;
    if (frame->video.encode == ZC_FRAME_ENC_H264 || frame->video.encode == ZC_FRAME_ENC_H265 ||
        frame->video.encode == ZC_FRAME_ENC_AAC) {
        if (-1 == m_streamid[frame->type]) {
            avtype = g_codeidTab[frame->video.encode];
            m_streamid[frame->type] = mpeg_ts_add_stream(m_ts, avtype, NULL, 0);
        }
        return m_streamid[frame->type];
    }

    return -1;
}

int CTsMuxer::_packetTs(zc_frame_t *frame) {
    int ret = 0;
    ret = _checkPts(frame);
    if (ret < 0) {
        return 0;
    }

    int stream = _tsAddStream(frame);
    if (stream < 0) {
        return 0;
    }
    int keyflag = frame->type == ZC_STREAM_VIDEO ? frame->keyflag : 0;
    ret = mpeg_ts_write(m_ts, stream, keyflag ? 1 : 0, frame->pts * 90, frame->pts * 90, frame->data, frame->size);
    if (ret < 0) {
        LOG_ERROR("tswrite error,ret:%d,type,%d,encode:%d,size:%u,pts:%llu", ret, frame->type, frame->video.encode,
                  frame->size, frame->pts);
        return 0;
    }
    m_pts[frame->type] = frame->pts;
    m_trace.TraceStream(frame);
    return 0;
}

int CTsMuxer::_getDate2PacketTs(CShmStreamR *stream) {
    unsigned int ret = 0;

    zc_frame_t *pframe = (zc_frame_t *)m_framebuf;
    // if (stream->Len() > sizeof(zc_frame_t)) {
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

#if 0  // ZC_DEBUG
       // debug info
        if (pframe->keyflag) {
            uint64_t now = zc_system_time();
            LOG_TRACE("rtsp:pts:%u,utc:%u,now:%u,len:%d,cos:%dms", pframe->pts, pframe->utc, now, pframe->size,
                      now - pframe->utc);
        }
#endif

        // packet ts
        if (_packetTs(pframe) < 0) {
            LOG_WARN("process into");
            return -1;
        }
    }

    return 0;
}

int CTsMuxer::_packetProcess() {
    LOG_WARN("_packetProcess into");
    CEpoll ep{100};  // set timeout 100ms,for rtspsource thread exit
    int ret = 0;

    if (!ep.Create()) {
        LOG_ERROR("epoll create error");
        return -1;
    }
    auto iter = m_vector.begin();
    for (; iter != m_vector.end(); ++iter) {
        int evfd = -1;
        if ((*iter) && (evfd = (*iter)->GetEvFd()) > 0) {
            LOG_WARN("epoll add fd[%d] ptr[%p]", evfd, (*iter));
            ep.Add(evfd, EPOLLIN | EPOLLET, (*iter));
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
                    if (_getDate2PacketTs(stream) < 0) {
                        m_status = ts_status_err;
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

int CTsMuxer::process() {
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