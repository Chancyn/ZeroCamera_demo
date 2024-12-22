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

#include "zc_frame.h"
// #include "zc_h26x_sps_parse.h"
#include "zc_basic_stream.h"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"
#include "ZcType.hpp"

#define DEBUG_DUMP (0)
// userspace modify................

// userspace modify................
namespace zc {
CShmStreamW::CShmStreamW(unsigned int size, const char *name, unsigned char chn, stream_puting_cb puting_cb, void *u)
    : CShmFIFO(size, name, chn, true), m_puting_cb(puting_cb), m_u(u) {
    ZC_ASSERT(name != nullptr);
    if (name[0] == 'v') {
        m_type = ZC_STREAM_VIDEO;
        m_magic = ZC_FRAME_VIDEO_MAGIC;
    } else if (name[0] == 'a') {
        m_type = ZC_STREAM_AUDIO;
        m_magic = ZC_FRAME_AUDIO_MAGIC;
    } else if (name[0] == 'm') {
        m_type = ZC_STREAM_META;
        m_magic = ZC_FRAME_META_MAGIC;
    } else {
        ZC_ASSERT(0);
    }
}

CShmStreamW::~CShmStreamW() {}

// put for hi_venc_stream, 1.first put hdr, 2.stream data PutAppending by m_puting_cb
unsigned int CShmStreamW::Put(const unsigned char *buffer, unsigned int len, void *stream) {
    unsigned int ret = 0;
    ShareLock();

    zc_frame_t *frame = (zc_frame_t *)buffer;

    frame->magic = m_magic;  // set frame magic
#if DEBUG_DUMP
    if (frame->keyflag) {
        LOG_TRACE("put encode:%u, size:%u", frame->video.encode, frame->size);
    }
#endif
    setLatestpos(frame->keyflag);
    ret = _put(buffer, len);

    // callback function need call puting to put framedata append
    if (m_puting_cb && stream) {
        ret += m_puting_cb(m_u, stream);
    }
    // put last data, write evfifo
    _putev();
    ShareUnlock();

    return ret;
}

// donot lock, puting_cb,callback function need call PutAppending to put framedata append
unsigned int CShmStreamW::PutAppending(const unsigned char *buffer, unsigned int len) {
    return _put(buffer, len);
}

CShmStreamR::CShmStreamR(unsigned int size, const char *name, unsigned char chn) : CShmFIFO(size, name, chn, false) {
    ZC_ASSERT(name != nullptr);
    if (name[0] == 'v') {
        m_type = ZC_STREAM_VIDEO;
        m_magic = ZC_FRAME_VIDEO_MAGIC;
        m_framemaxlen = ZC_STREAM_MAXFRAME_SIZE;
    } else if (name[0] == 'a') {
        m_type = ZC_STREAM_AUDIO;
        m_magic = ZC_FRAME_AUDIO_MAGIC;
        m_framemaxlen = ZC_STREAM_MAXFRAME_SIZE_A;
    } else if (name[0] == 'm') {
        m_type = ZC_STREAM_META;
        m_magic = ZC_FRAME_META_MAGIC;
        m_framemaxlen = ZC_STREAM_MAXFRAME_SIZE_M;
    } else {
        ZC_ASSERT(0);
    }
}

CShmStreamR::~CShmStreamR() {}

unsigned int CShmStreamR::_getLatestFrameHdr(unsigned char *buffer, unsigned int hdrlen, bool keyflag) {
    unsigned int pos = 0;
    // get latest frame pos
    pos = getLatestPos(keyflag);
    // set out pos = latest frame pos
    setLatestOutpos(pos);
    unsigned int ret = _get(buffer, hdrlen);
    unsigned int *magicb = (unsigned int *)buffer;
    if (keyflag && ((*magicb) != m_magic)) {
        LOG_ERROR("get keyframe error magic[0x%x]!=[0x%x], try get latest frame", (*magicb), m_magic);
        pos = getLatestPos(false);
        setLatestOutpos(pos);
        ret = _get(buffer, hdrlen);
    }

    ZC_ASSERT(ret == hdrlen);
    LOG_WARN("get latest IDR frame hdr ret[%u]", ret);
    return ret;
}

bool CShmStreamR::_praseFrameInfo(zc_frame_userinfo_t &info, zc_frame_t *frame) {
    bool flags = false;

    if (frame->video.encode == ZC_FRAME_ENC_H264 || frame->video.encode == ZC_FRAME_ENC_H265) {
        unsigned char *pos = frame->data;
        unsigned char naluval = 0;
        unsigned char prefixlen = 0x01 == frame->data[2] ? 3 : 4;
        // ZC_ASSERT(frame->keyflag); // TODO(zhoucc):
        if (frame->video.nalunum == 0) {
            LOG_WARN("no naluinfo, prase frame->size:%d", frame->size);
            zc_h26x_nalu_info_t tmp;
            memset(&tmp, 0, sizeof(zc_h26x_nalu_info_t));
            frame->video.nalunum = zc_h26x_parse_nalu(frame->data, frame->size, &tmp, frame->video.encode);
            for (unsigned int i = 0; i < frame->video.nalunum; i++) {
                if (tmp.nalus[i].size > 0) {
                    frame->video.nalu[i] = tmp.nalus[i].size;
                }
            }
        }
        info.encode = frame->video.encode;
        info.type = ZC_STREAM_VIDEO;
        info.vinfo.width = frame->video.width;
        info.vinfo.height = frame->video.height;
        info.vinfo.nalunum = 0;
        for (unsigned int i = 0; i < frame->video.nalunum; i++) {
            pos += prefixlen;
            naluval = *pos;
            LOG_WARN("IDR frameinfo i:%d,naluval:%u,len:%u, prefixlen:%u", i, naluval, frame->video.nalu[i], prefixlen);
            if (frame->video.nalu[i] > 0 && frame->video.nalu[i] <= ZC_FRAME_NALU_BUFF_MAX_SIZE) {
                info.vinfo.nalu[info.vinfo.nalunum].type = zc_h26x_nalu_val2type(naluval, frame->video.encode);
                info.vinfo.nalu[info.vinfo.nalunum].size = frame->video.nalu[i];
                memcpy(info.vinfo.nalu[info.vinfo.nalunum].data, pos, frame->video.nalu[i]);
                flags = true;
                info.vinfo.nalunum++;
            }
            pos += frame->video.nalu[i];
            if (pos >= frame->data + frame->size) {
                break;
            }
        }
        LOG_TRACE("prase frame len:%u, num:%d, flags:%d", frame->size, info.vinfo.nalunum, flags);
    } else if (frame->audio.encode == ZC_FRAME_ENC_AAC) {
        zc_mpeg4_aac_t aac;
        zc_mpeg4_aac_adts_load(frame->data, frame->size, &aac);
        info.encode = ZC_FRAME_ENC_AAC;
        info.type = ZC_STREAM_AUDIO;
        info.ainfo.channels = aac.channels;
        info.ainfo.sample_bits = ZC_AUDIO_SAMPLE_BIT_16;
        info.ainfo.sample_rate = aac.sampling_frequency;
        memcpy(info.ainfo.adts, frame->data, 7);
        zc_debug_dump_binstream("audio_adts", ZC_STREAM_AUDIO, frame->data, frame->size, 64);
        flags = true;
        LOG_TRACE("prase frame len:%u, channels:%d,sample_bits:%d,sample_rate:%d ", frame->size, info.ainfo.channels,
                  info.ainfo.sample_bits, info.ainfo.sample_rate);
    }

    return flags;
}

bool CShmStreamR::_getLatestFrameInfo(zc_frame_userinfo_t &info) {
    bool ret = false;
    unsigned int hdrlen = sizeof(zc_frame_t);
    unsigned char buffer[hdrlen + m_framemaxlen];
    zc_frame_t *frame = (zc_frame_t *)buffer;
    unsigned int framelen = 0;
    ShareLock();
    _findSkip2LatestPos(m_type == ZC_STREAM_VIDEO ? true : false);
    unsigned int len = _preget(buffer, hdrlen);
    if (frame->magic != m_magic) {
        LOG_ERROR("empty frame magic:0x%08x != 0x%08x", frame->magic, m_magic);
        goto _err;
    }

    framelen = frame->size;
    len = _preget(buffer, framelen+hdrlen);
    if (len != framelen+hdrlen) {
        LOG_ERROR("get frame error len:%u != %u", len, framelen);
        goto _err;
    }

    // get streaminfo by key frame
    if (!_praseFrameInfo(info, frame)) {
        LOG_ERROR("_praseFrameInfo error");
        goto _err;
    }
    ret = true;
_err:
    ShareUnlock();
    LOG_WARN("get latest IDR frameinfo ret[%u]", ret);
    return ret;
}

// put for hi_venc_stream, 1.first put hdr, 2.stream data GetAppending by m_puting_cb
unsigned int CShmStreamR::Get(unsigned char *buffer, unsigned int buflen, unsigned int hdrlen) {
    unsigned int framelen = 0;
    unsigned int ret = 0;
    ShareLock();
    ret = _get(buffer, hdrlen);
    ZC_ASSERT(ret == hdrlen);
    unsigned int *magicb = (unsigned int *)buffer;
    zc_frame_t *frame = (zc_frame_t *)buffer;
    framelen = frame->size;
    // ZC_ASSERT((*magicb) == magic);
    if (frame->magic != m_magic) {
        // get latest frame
        LOG_ERROR("magic[0x%x]!=[0x%x], get latest IDR frame", (*magicb), m_magic);
        _findSkip2LatestPos(m_type == ZC_STREAM_VIDEO ? true : false);
        ret = _get(buffer, hdrlen);
        if (frame->magic != m_magic) {
            LOG_ERROR("empty frame, [0x%x]!=[0x%x]", frame->magic, m_magic);
            ShareUnlock();
            return 0;
        }
    }

    framelen = frame->size;
    // LOG_ERROR("buflen[%u]hdrlen[%u]framelen[%u]ret[%u]", buflen, hdrlen, framelen, ret);
    ZC_ASSERT(buflen >= hdrlen + framelen);
    ret = _get(buffer + hdrlen, framelen);
    ZC_ASSERT(ret == framelen);
    ShareUnlock();

    return ret + hdrlen;
}

bool CShmStreamR::GetStreamInfo(zc_frame_userinfo_t &info, bool skip2lastest /*=false*/) {
    bool ret = false;
    if (!skip2lastest) {
        // just get info;do't jump to lastest frame
        unsigned int len = 0;
        len = getUserData((unsigned char *)&info, sizeof(zc_frame_userinfo_t));
        if (len == sizeof(zc_frame_userinfo_t)) {
            LOG_WARN("get userdata frameinfo ok");
            return ret;
        }
    }

    ret = _getLatestFrameInfo(info);
    if (!ret) {
        LOG_ERROR("get userdata frameinfo error, try get latest ret:%d", ret);
    }
    return ret;
}

bool CShmStreamR::_findSkip2LatestPos(bool key) {
    zc_frame_t frame;
    unsigned int hdrlen = sizeof(zc_frame_t);
    unsigned int pos = 0;
    // get latest frame pos
    pos = getLatestPos(key);
    // set out pos = latest frame pos
    setLatestOutpos(pos);
    unsigned int ret = _preget((unsigned char *)&frame, hdrlen);
    if (frame.magic != m_magic) {
        LOG_WARN("ret:%u, find type:%d, keyframe error magic[0x%x]!=[0x%x], try get latest frame", ret, m_type, frame.magic,
                 m_magic);
        if (key) {
            pos = getLatestPos(false);
            setLatestOutpos(pos);
            ret = _preget((unsigned char *)&frame, hdrlen);
        }
    }

    LOG_WARN("find type:%d, result:%d, magic[0x%x]-[0x%x]", m_type, frame.magic == m_magic, frame.magic, m_magic);
    if (frame.magic != m_magic) {
        // TODO(zhoucc): goto end
        LOG_ERROR("find type:%d, result:%d, magic[0x%x]-[0x%x]", m_type, frame.magic == m_magic, frame.magic, m_magic);
    }
    return frame.magic == m_magic;
}

void CShmStreamR::Skip2LatestPos(bool key) {
    ShareLock();
    _findSkip2LatestPos(key);
    ShareUnlock();
    return;
}

}  // namespace zc
