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
#include "zc_proc.h"

#include "Epoll.hpp"
#include "Thread.hpp"
#include "ZcFlvMuxer.hpp"
#include "ZcType.hpp"

namespace zc {
#define ZC_SERVERNAME "ZeroCamrea(zhoucc)"  //
#define MKBETAG(a, b, c, d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
// UB [4]; Codec Identifier.
enum flv_videocodecid_e {
    flv_vid_vp6 = 4,        // On2 VP6
    flv_vid_vp6_alpha = 5,  // On2 VP6 with alpha channel
    flv_vid_h264 = 7,       // avc
    flv_vid_h265 = 12,      // 国内扩展

    // 增强型rtmp FourCC
    flv_vid_fourcc_vp9 = MKBETAG('v', 'p', '0', '9'),
    flv_fourcc_av1 = MKBETAG('a', 'v', '0', '1'),
    flv_fourcc_hevc = MKBETAG('h', 'v', 'c', '1')
};

// UB [4]; Format of SoundData
enum flv_audiocodecid_e {
    /**
    0 = Linear PCM, platform endian
    1 = ADPCM
    2 = MP3
    3 = Linear PCM, little endian
    4 = Nellymoser 16 kHz mono
    5 = Nellymoser 8 kHz mono
    6 = Nellymoser
    7 = G.711 A-law logarithmic PCM
    8 = G.711 mu-law logarithmic PCM
    9 = reserved
    10 = AAC
    11 = Speex
    14 = MP3 8 kHz
    15 = Device-specific sound
    */
    flv_aid_g711a = 7,
    flv_aid_g711u = 8,
    flv_aid_aac = 10,
    flv_aid_opus = 13  // 国内扩展
};

static inline uint32_t getFlvCodeId(zc_frame_enc_e enc) {
    switch (enc) {
    case ZC_FRAME_ENC_H264:
        return flv_vid_h264;
    case ZC_FRAME_ENC_H265:
        return flv_vid_h265;
    case ZC_FRAME_ENC_AAC:
        return flv_aid_aac;
    default:
        break;
    }

    return 0;
}

CFlvMuxer::CFlvMuxer()
    : Thread("flvmuxer"), m_Idr(false), m_status(flv_status_init), m_pts(0), m_apts(0), m_flv(nullptr) {
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

    // check auido trackinfo
    zc_audio_trackinfo_t *atrack = &m_info.streaminfo.tracks[ZC_MEDIA_TRACK_AUDIO].atinfo;
    if (atrack->channels) {
        atrack->channels = atrack->channels ? atrack->channels : ZC_AUDIO_CHN;
        atrack->sample_bits = atrack->sample_bits ? atrack->sample_bits : ZC_AUDIO_SAMPLE_BIT_16;
        atrack->sample_rate = atrack->sample_rate ? atrack->sample_rate : ZC_AUDIO_FREQUENCE;
    } else {
        atrack->channels = ZC_AUDIO_CHN;
        atrack->sample_bits = ZC_AUDIO_SAMPLE_BIT_16;
        atrack->sample_rate = ZC_AUDIO_FREQUENCE;
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

    metadata.audiocodecid = getFlvCodeId((zc_frame_enc_e)m_info.streaminfo.tracks[ZC_MEDIA_TRACK_AUDIO].encode);
    metadata.audiodatarate = 16.1;
    metadata.audiosamplerate = m_info.streaminfo.tracks[ZC_MEDIA_TRACK_AUDIO].atinfo.sample_rate;  // 480000
    metadata.audiosamplesize = m_info.streaminfo.tracks[ZC_MEDIA_TRACK_AUDIO].atinfo.sample_bits * 8;
    metadata.stereo = m_info.streaminfo.tracks[ZC_MEDIA_TRACK_AUDIO].atinfo.channels > 1 ? true : false;
    metadata.videocodecid = flv_vid_h264;// getFlvCodeId((zc_frame_enc_e)m_info.streaminfo.tracks[ZC_MEDIA_TRACK_VIDEO].encode);
    metadata.videodatarate = 64.0;
    metadata.framerate = m_info.streaminfo.tracks[ZC_MEDIA_TRACK_VIDEO].vtinfo.fps;
    metadata.width = m_info.streaminfo.tracks[ZC_MEDIA_TRACK_VIDEO].vtinfo.width;
    metadata.height = m_info.streaminfo.tracks[ZC_MEDIA_TRACK_VIDEO].vtinfo.height;
    strncpy(metadata.server, ZC_SERVERNAME, sizeof(metadata.server));
    snprintf(metadata.server_ver, sizeof(metadata.server_ver), "%s", ZcGetVersionBuildDateStr());

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

#if 0  // ZC_DEBUG
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