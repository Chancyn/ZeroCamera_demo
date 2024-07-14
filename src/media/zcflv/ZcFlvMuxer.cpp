// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "flv-muxer.h"
#include "flv-proto.h"
#include "flv-reader.h"
// #include "sys/system.h"
#include "zc_frame.h"
#include "zc_log.h"

#include "Epoll.hpp"
#include "Thread.hpp"
#include "ZcFlvMuxer.hpp"
#include "ZcType.hpp"

namespace zc {
CFlvMuxer::CFlvMuxer() : m_Idr(false), m_status(flv_status_init), m_pts(0), m_apts(0), m_flv(nullptr) {
    memset(&m_info, 0, sizeof(m_info));
    m_vector.clear();
}

CFlvMuxer::~CFlvMuxer() {
    Destroy();
}

bool CFlvMuxer::destroyStream() {
    auto iter = m_vector.begin();
    for (; iter != m_vector.end();) {
        ZC_SAFE_DELETE(*iter);
        iter = m_vector.erase(iter);
    }
    m_vector.clear();

    return true;
}

bool CFlvMuxer::createStream() {
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
        m_vector.push_back(fiforeader);
    }

    if (m_vector.size() <= 0) {
        LOG_ERROR("no stream error");
        return false;
    }

    return true;
}

bool CFlvMuxer::Create(const zc_flvmuxer_info_t &info) {
    if (m_flv) {
        return false;
    }

    memcpy(&m_info, &info, sizeof(m_info));
    if (!createStream()) {
        LOG_ERROR("create error");
        goto _err;
    }

    m_flv = flv_muxer_create(m_info.onflvpacketcb, m_info.Context);
    if (!m_flv) {
        LOG_ERROR("create error");
        goto _err;
    }

    LOG_TRACE("create ok");
    return true;
_err:
    destroyStream();
    m_flv = nullptr;
    LOG_ERROR("create error");
    return false;
}

bool CFlvMuxer::Destroy() {
    if (!m_flv) {
        return false;
    }

    Stop();
    flv_muxer_destroy(m_flv);
    destroyStream();
    m_flv = nullptr;
    LOG_TRACE("destroy ok");
    return true;
}

bool CFlvMuxer::Start() {
    if (!m_flv) {
        return false;
    }

    Thread::Start();
    m_status = flv_status_init;
    return true;
}

bool CFlvMuxer::Stop() {
    Thread::Stop();
    return false;
}

int CFlvMuxer::_fillFlvMuxerMeta() {
    // TODO(zhoucc): fill metadata hdr
    struct flv_metadata_t metadata;
    metadata.audiocodecid = 4;
    metadata.audiodatarate = 16.1;
    metadata.audiosamplerate = 48000;
    metadata.audiosamplesize = 16;
    metadata.stereo = TRUE;
    metadata.videocodecid = 7;
    metadata.videodatarate = 64.0;
    metadata.framerate = 30;
    metadata.width = 1920;
    metadata.height = 1080;

    flv_muxer_metadata(m_flv, &metadata);
    return 0;
}

int CFlvMuxer::_packetFlv(zc_frame_t *frame) {
    if (frame->video.encode == ZC_FRAME_ENC_H264) {
        if (!m_pts) {
            _fillFlvMuxerMeta();
            m_pts = frame->pts;
        }

        // sps-pps-vcl
        if (flv_muxer_avc(m_flv, frame->data, frame->size, frame->pts - m_pts, frame->pts - m_pts) < 0) {
            LOG_ERROR("push error, flv_muxer_avc err.\n");
            return -1;
        }
    } else if (frame->video.encode == ZC_FRAME_ENC_H265) {
        if (!m_pts) {
            _fillFlvMuxerMeta();
            m_pts = frame->pts;
        }

        // https://github.com/ksvc/FFmpeg/wiki
        // https://ott.dolby.com/codec_test/index.html
        // sps-pps-vcl
        if (flv_muxer_hevc(m_flv, frame->data, frame->size, frame->pts - m_pts, frame->pts - m_pts) < 0) {
            LOG_ERROR("flv_muxer_hevc err.\n");
            return -1;
        }
    } else if (frame->audio.encode == ZC_FRAME_ENC_AAC) {
        if (m_pts) {
            if (!m_apts) {
                LOG_INFO("A rec->type:%d, seq:%d, utc:%u, size:%d\n", frame->type, frame->seq, frame->utc, frame->size);
                m_apts = frame->pts;
            }

            if (flv_muxer_aac(m_flv, frame->data, frame->size, frame->pts - m_apts, frame->pts - m_apts) < 0) {
                LOG_ERROR("flv_muxer_hevc err.\n");
                return -1;
            }
        }
    }

    return 0;
}

int CFlvMuxer::_getDate2PacketFlv(CShmStreamR *stream) {
    int ret = 0;

    zc_frame_t *pframe = (zc_frame_t *)m_framebuf;
    if (stream->Len() > sizeof(zc_frame_t)) {
        ret = stream->Get(m_framebuf, sizeof(m_framebuf), sizeof(zc_frame_t), ZC_FRAME_VIDEO_MAGIC);
        if (ret < sizeof(zc_frame_t)) {
            return -1;
        }

        // first IDR frame
        if (!m_Idr) {
            if (!pframe->keyflag) {
                LOG_WARN("drop , need IDR frame");
                return 0;
            } else {
                m_Idr = true;
            }
        }

#if ZC_DEBUG
        // debug info
        if (pframe->keyflag) {
            struct timespec _ts;
            clock_gettime(CLOCK_MONOTONIC, &_ts);
            unsigned int now = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
            LOG_TRACE("rtsp:pts:%u,utc:%u,now:%u,len:%d,cos:%dms", pframe->pts, pframe->utc, now, pframe->size,
                      now - pframe->utc);
        }
#endif

        // packet flv
        if (_packetFlv(pframe) < 0) {
            LOG_WARN("process into\n");
            return -1;
        }
    }

    return 0;
}

int CFlvMuxer::_packetProcess() {
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
                    if (_getDate2PacketFlv(stream) < 0) {
                        m_status = flv_status_err;
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

int CFlvMuxer::process() {
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