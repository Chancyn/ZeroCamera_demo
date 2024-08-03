// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/syslog.h>
#include <sys/types.h>
#include <unistd.h>

#include "fmp4-writer.h"
#include "mov-format.h"
#include "mp4-writer.h"
#include "mpeg4-aac.h"
#include "mpeg4-avc.h"
#include "mpeg4-hevc.h"
#include "zc_h26x_sps_parse.h"
// #include "sys/system.h"
#include "zc_frame.h"
#include "zc_log.h"

#include "Epoll.hpp"
#include "Thread.hpp"
#include "ZcFmp4Muxer.hpp"
#include "ZcType.hpp"
#include "zc_macros.h"

namespace zc {
#define ZC_FMP4MUXER_DEBUG_DUMP 1

CFmp4Muxer::CFmp4Muxer()
    : Thread("fmp4buf"), m_Idr(false), m_status(fmp4_status_init), m_pts(0), m_apts(0), m_fmp4(nullptr),
      m_movio(nullptr) {
    memset(&m_info, 0, sizeof(m_info));
    m_vector.clear();
}

CFmp4Muxer::~CFmp4Muxer() {
    Destroy();
}

bool CFmp4Muxer::destroyStream() {
    auto iter = m_vector.begin();
    for (; iter != m_vector.end();) {
        ZC_SAFE_DELETE(*iter);
        iter = m_vector.erase(iter);
    }
    m_vector.clear();

    return true;
}

bool CFmp4Muxer::createStream() {
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
        zc_frame_userinfo_t frameinfo;
        // Skip2LatestPos();
        if (fiforeader->GetStreamInfo(frameinfo, true)) {
            LOG_WARN("Get Streaminfo/jump2latest ok");
        }
        m_vector.push_back(fiforeader);
    }

    if (m_vector.size() <= 0) {
        LOG_ERROR("no stream error");
        return false;
    }

    return true;
}

bool CFmp4Muxer::Create(const zc_fmp4muxer_info_t &info) {
    if (m_fmp4) {
        return false;
    }

    memcpy(&m_info, &info, sizeof(m_info));
    if (!createStream()) {
        LOG_ERROR("create error");
        goto _err;
    }

    // debug
    // m_movio = CMovIoFac::CMovIoCreate(info.movinfo);
    m_movio = CMovIoFac::CMovIoCreate(fmp4_movio_buffile, &info.movinfo.uninfo);
    if (!m_movio) {
        LOG_ERROR("CMovIoCreate error");
        goto _err;
    }

    m_fmp4 = fmp4_writer_create(m_movio->GetIoInfo(), m_movio, MOV_FLAG_FASTSTART | MOV_FLAG_SEGMENT);
    if (!m_fmp4) {
        LOG_ERROR("create error");
        goto _err;
    }

    // init track id
    for (unsigned int i = 0; i < _SIZEOFTAB(m_trackid); i++) {
        m_trackid[i] = -1;
    }

    LOG_TRACE("create ok");
    return true;
_err:
    ZC_SAFE_DELETE(m_movio);
    destroyStream();
    m_fmp4 = nullptr;
    LOG_ERROR("create error");
    return false;
}

bool CFmp4Muxer::Destroy() {
    if (!m_fmp4) {
        return false;
    }

    Stop();
    fmp4_writer_destroy(m_fmp4);
    ZC_SAFE_DELETE(m_movio);
    destroyStream();
    m_fmp4 = nullptr;
    LOG_TRACE("destroy ok");
    return true;
}

bool CFmp4Muxer::Start() {
    if (!m_fmp4) {
        return false;
    }

    // find idr;

    Thread::Start();
    m_status = fmp4_status_init;
    return true;
}

bool CFmp4Muxer::Stop() {
    Thread::Stop();
    return false;
}

int CFmp4Muxer::_write2Fmp4(zc_frame_t *pframe) {
    int update = 0;
    int vcl = 0;
    int n = 0;
    int ret = -1;
    int keyflag = 0;
    uint8_t extra_data[64 * 1024];
    int segment = 0;
    if (pframe->type == ZC_STREAM_VIDEO) {
        keyflag = pframe->keyflag;
        if (pframe->video.encode == ZC_FRAME_ENC_H264) {
            struct mpeg4_avc_t avc = {0};
            n = h264_annexbtomp4(&avc, pframe->data, pframe->size, m_framemp4buf, sizeof(m_framemp4buf), &vcl, &update);
            if (m_trackid[ZC_STREAM_VIDEO] == -1) {
                LOG_WARN("H264 [size:%d, n:%d, vlc:%d]", pframe->size, n, vcl);
                if (avc.nb_sps < 1 || avc.sps[0].bytes < 4) {
                    LOG_WARN("H264 skip video wait for key frame [idc:%d, nb_sps:%d, sps:%d]", avc.chroma_format_idc,
                             avc.nb_sps, avc.sps[0].bytes);
                    return 0;
                }
                if (pframe->keyflag && (pframe->video.width == 0 || pframe->video.height == 0)) {
                    zc_h26x_sps_info_t spsinfo = {0};
                    zc_debug_dump_binstream(__FUNCTION__, ZC_FRAME_ENC_H264, avc.sps[0].data, avc.sps[0].bytes,
                                            avc.sps[0].bytes);
                    if (zc_h264_sps_parse(avc.sps[0].data, avc.sps[0].bytes, &spsinfo) == 0) {
                        pframe->video.width = spsinfo.width;    // picture width;
                        pframe->video.height = spsinfo.height;  // picture height;
                        LOG_WARN("prase wh [size:%d, wh:%hu:%hu]", pframe->video.width, pframe->video.height);
                    }
                }
                LOG_INFO("fmp4 => add H264 info->v.w:%d, info->v.h:%d", pframe->video.width, pframe->video.height);
                int extra_data_size = mpeg4_avc_decoder_configuration_record_save(&avc, extra_data, sizeof(extra_data));
                assert(extra_data_size > 0);  // check buffer length

                m_trackid[ZC_STREAM_VIDEO] = fmp4_writer_add_video(m_fmp4, MOV_OBJECT_H264, pframe->video.width,
                                                                   pframe->video.height, extra_data, extra_data_size);
                LOG_WARN("H264 [size:%d, n:%d, vlc:%d, track:%d, extra:%d]", pframe->size, n, vcl,
                         m_trackid[ZC_STREAM_VIDEO], extra_data_size);
                fmp4_writer_init_segment(m_fmp4);
            }

            if (m_trackid[ZC_STREAM_VIDEO] != -1) {
                // LOG_TRACE("H264 [size:%d, n:%d, vlc:%d, track:%d]", pframe->size, n, vcl,
                // m_trackid[ZC_STREAM_VIDEO]);
                ret = fmp4_writer_write(m_fmp4, m_trackid[ZC_STREAM_VIDEO], m_framemp4buf, n, pframe->pts, pframe->pts,
                                        pframe->keyflag ? MOV_AV_FLAG_KEYFREAME : 0);
            }
        } else if (pframe->video.encode == ZC_FRAME_ENC_H265) {
            struct mpeg4_hevc_t hevc = {0};

            n = h265_annexbtomp4(&hevc, pframe->data, pframe->size, m_framemp4buf, sizeof(m_framemp4buf), &vcl,
                                 &update);
            if (m_trackid[ZC_STREAM_VIDEO] == -1) {
                if (hevc.numOfArrays < 1) {
                    LOG_WARN("H265 skip wait for key frame");
                    return 0;
                }
                // zc_debug_dump_binstream(__FUNCTION__, ZC_FRAME_ENC_H265, pframe->data, pframe->size, 64);

                // LOG_WARN("H265 numnalu:%u, keyflag:%u, size:%u", hevc.numOfArrays, pframe->keyflag, pframe->size);
                if (pframe->keyflag && (pframe->video.width == 0 || pframe->video.height == 0)) {
                    zc_h26x_sps_info_t spsinfo = {0};
                    for (unsigned int i = 0; i < _SIZEOFTAB(hevc.nalu) && i < ZC_FRAME_NALU_MAXNUM; i++) {
                        zc_debug_dump_binstream(__FUNCTION__, ZC_FRAME_ENC_H265, hevc.nalu[i].data, hevc.nalu[i].bytes,
                                                hevc.nalu[i].bytes);
                        if (hevc.nalu[i].type == H265_NAL_UNIT_SPS &&
                            zc_h265_sps_parse(hevc.nalu[i].data, hevc.nalu[i].bytes, &spsinfo) == 0) {
                            pframe->video.width = spsinfo.width;    // picture width;
                            pframe->video.height = spsinfo.height;  // picture height;
                            LOG_WARN("prase wh [size:%d, wh:%hu:%hu]", pframe->video.width, pframe->video.height);
                            break;
                        }
                    }
                }
                int extra_data_size =
                    mpeg4_hevc_decoder_configuration_record_save(&hevc, extra_data, sizeof(extra_data));
                assert(extra_data_size > 0);  // check buffer length
                LOG_INFO("fmp4 => add H265 info->v.w:%d, info->v.h:%d", pframe->video.width, pframe->video.height);
                m_trackid[ZC_STREAM_VIDEO] = fmp4_writer_add_video(m_fmp4, MOV_OBJECT_HEVC, pframe->video.width,
                                                                   pframe->video.height, extra_data, extra_data_size);
                fmp4_writer_init_segment(m_fmp4);
            }

            if (m_trackid[ZC_STREAM_VIDEO] != -1) {
                ret = fmp4_writer_write(m_fmp4, m_trackid[ZC_STREAM_VIDEO], m_framemp4buf, n, pframe->pts, pframe->pts,
                                        pframe->keyflag ? MOV_AV_FLAG_KEYFREAME : 0);
            }
        }
    } else if (pframe->type == ZC_STREAM_AUDIO) {
        int rate = 1;
        struct mpeg4_aac_t aac;
        uint8_t extra_data[64 * 1024];
        mpeg4_aac_adts_load((const uint8_t *)pframe->data, pframe->size, &aac);

        if (m_trackid[ZC_STREAM_AUDIO] == -1) {
            LOG_INFO("fmp4 => add AAC");
            int extra_data_size = mpeg4_aac_audio_specific_config_save(&aac, extra_data, sizeof(extra_data));
            rate = mpeg4_aac_audio_frequency_to((enum mpeg4_aac_frequency)aac.sampling_frequency_index);
            m_trackid[ZC_STREAM_AUDIO] = fmp4_writer_add_audio(m_fmp4, MOV_OBJECT_AAC, aac.channel_configuration, 16,
                                                               rate, extra_data, extra_data_size);
            fmp4_writer_init_segment(m_fmp4);
        }

        if (m_trackid[ZC_STREAM_AUDIO] != -1) {
            unsigned char *_buf = pframe->data;
            // mpeg4_aac_adts_frame_length();
            int framelen = ((_buf[3] & 0x03) << 11) | (_buf[4] << 3) | ((_buf[5] >> 5) & 0x07);
            // LOG_TRACE("AAC framelen:%d", framelen);
            ret = fmp4_writer_write(m_fmp4, m_trackid[ZC_STREAM_AUDIO], _buf + 7, framelen - 7, pframe->pts,
                                    pframe->pts, 0);
        }
    }

    // save segment,and get data to send
    segment = keyflag ? 1 : 0;
    if (segment && fmp4_writer_save_segment(m_fmp4) != 0) {
        LOG_ERROR("write,erro, pframe->size:%u", pframe->size);
        return ret;
    }

    if (m_info.onfmp4packetcb) {
        // get buf number,
        const uint8_t *data = nullptr;
        uint32_t len = 0;
        data = m_movio->GetDataBufPtr(len);
        if (data) {
            m_info.onfmp4packetcb(m_info.Context, keyflag, data, len, 0);
        }
    }

    if (segment) {
        m_movio->ResetDataBufPos();
    }
    return ret;
}

int CFmp4Muxer::_getDate2Write2Fmp4(CShmStreamR *stream) {
    int ret = 0;
    zc_frame_t *pframe = (zc_frame_t *)m_framebuf;
    // (stream->Len() > sizeof(zc_frame_t)) {
    while (State() == Running && (stream->Len() > sizeof(zc_frame_t))) {
        ret = stream->Get(m_framebuf, sizeof(m_framebuf), sizeof(zc_frame_t));
        if (ret < sizeof(zc_frame_t)) {
            return -1;
        }

#if 0  // no need anymore
       // first IDR frame
        if (!m_Idr) {
            if (!pframe->keyflag) {
                LOG_WARN("drop , need IDR frame");
                return 0;
            } else {
                m_Idr = true;
            }
        }
#endif

#if 1  // ZC_DEBUG
       // debug info
        if (pframe->keyflag) {
            struct timespec _ts;
            clock_gettime(CLOCK_MONOTONIC, &_ts);
            unsigned int now = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
            LOG_TRACE("fmp4:pts:%u,utc:%u,now:%u,len:%d,cos:%dms", pframe->pts, pframe->utc, now, pframe->size,
                      now - pframe->utc);
        }
#endif

        // packet flv
        if (_write2Fmp4(pframe) < 0) {
            LOG_WARN("process into");
            return -1;
        }

        ZC_MSLEEP(1);
    }

    return 0;
}

int CFmp4Muxer::_packetProcess() {
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
                    if (_getDate2Write2Fmp4(stream) < 0) {
                        m_status = fmp4_status_err;
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

int CFmp4Muxer::process() {
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