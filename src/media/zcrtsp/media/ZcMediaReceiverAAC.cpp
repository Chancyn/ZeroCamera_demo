// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <cstddef>
#include <stdio.h>
#include <string.h>

#include "sys/system.h"

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcMediaReceiverAAC.hpp"

namespace zc {
/*
// ISO-14496-3 adts_frame (p122)

adts_fixed_header()
{
    syncword;					12 bslbf
    ID;							1 bslbf
    layer;						2 uimsbf
    protection_absent;			1 bslbf
    profile_ObjectType;			2 uimsbf
    sampling_frequency_index;	4 uimsbf
    private_bit;				1 bslbf
    channel_configuration;		3 uimsbf
    original_copy;				1 bslbf
    home;						1 bslbf
}

adts_variable_header()
{
    copyright_identification_bit;		1 bslbf
    copyright_identification_start;		1 bslbf
    aac_frame_length;					13 bslbf
    adts_buffer_fullness;				11 bslbf
    number_of_raw_data_blocks_in_frame; 2 uimsbf
}
*/

// {0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0xFC};
static inline int adts_fixed_header(zc_aac_info_t *aac, char data[7], int len) {
    len += 7;  // framelen = dtshdrlen+datalen
    data[0] = 0xFF;
    data[1] = 0xF0 | (aac->id << 3) | (0x00 << 2) | 0x01;
    data[2] = ((aac->profile - 1) << 6) | ((aac->sampling_frequency_index & 0x0F) << 2) |
              ((aac->channel_configuration >> 2) & 0x01);
    // len:13bits len.bit[12-11] @data[3].bit[1-0]
    data[3] = ((aac->channel_configuration & 0x03) << 6) | ((len >> 11) & 0x03);
    // len:13bits len.bit[10-3] @data[4].bit[7-0]
    data[4] = (uint8_t)(len >> 3);
    // len:13bits len.bit[2-0] @data[5].bit[7-5]
    data[5] = ((len & 0x07) << 5) | 0x1F;
    data[6] = 0xFC;

    LOG_TRACE("ADTS hdr[%02X,%02X,%02X,%02X,%02X,%02X,%02X]", data[0], data[1], data[2], data[3], data[4], data[5],
              data[6]);
    return 0;
}

typedef struct {
    unsigned int len;
    const uint8_t *ptr;
} audio_raw_frame_t;

// bufsize = zc_frame_t hdr + ADTS_HDR
CMediaReceiverAAC::CMediaReceiverAAC(int shmtype, int chn)
    : CMediaReceiver(ZC_MEDIA_TRACK_AUDIO, ZC_MEDIA_CODE_AAC, shmtype, chn, 7) {
    m_frame = (zc_frame_t *)m_framebuf;
    memset(m_frame, 0, sizeof(zc_frame_t));
    m_frame->magic = ZC_FRAME_AUDIO_MAGIC;
    m_frame->type = ZC_STREAM_AUDIO;
    m_frame->audio.encode = ZC_FRAME_ENC_AAC;

    m_info.profile = MPEG4_AAC_MAIN;
    m_info.sampling_frequency_index = MPEG4_AAC_48000;
    m_info.channel_configuration = 2;
    // init hdr
    adts_fixed_header(&m_info, m_dtshdr, 0);
}

CMediaReceiverAAC::~CMediaReceiverAAC() {}

bool CMediaReceiverAAC::Init(void *pinfo) {
    // TODO(zhoucc): update dts hdr
    if (m_shmtype == ZC_SHMSTREAM_PUSH) {
        // push-server recv stream
        m_fifowriter =
            new CShmStreamW(ZC_STREAM_AUDIO_SIZE, ZC_STREAM_AUDIOPUSH_SHM_PATH, m_chn, putingCb, this);
    } else {
        // pull-cli recv stream,
        m_fifowriter =
            new CShmStreamW(ZC_STREAM_AUDIO_SIZE, ZC_STREAM_AUDIOPULL_SHM_PATH, m_chn, putingCb, this);
    }
    if (!m_fifowriter) {
        LOG_ERROR("Create m_fifowriter");
        goto _err;
    }

    if (!m_fifowriter->ShmAlloc()) {
        LOG_ERROR("ShmAlloc error");
        goto _err;
    }

    // set create flag
    m_create = true;
    LOG_TRACE("Create ok ,AAC");
    return true;
_err:
    LOG_ERROR("Create error AAC");
    ZC_SAFE_DELETE(m_fifowriter);
    return false;
}

// frame
void CMediaReceiverAAC::_framelenUpdateADTSHdr(int len) {
    len += 7;  // framelen = dtshdrlen+datalen
    // len:13bits len.bit[12-11] @data[3].bit[1-0]
    m_dtshdr[3] = ((m_info.channel_configuration & 0x03) << 6) | ((len >> 11) & 0x03);
    // len:13bits len.bit[10-3] @data[4].bit[7-0]
    m_dtshdr[4] = (uint8_t)(len >> 3);
    // len:13bits len.bit[2-0] @data[5].bit[7-5]
    m_dtshdr[5] = ((len & 0x07) << 5) | 0x1F;
    return;
}

unsigned int CMediaReceiverAAC::putingCb(void *u, void *stream) {
    CMediaReceiverAAC *self = reinterpret_cast<CMediaReceiverAAC *>(u);
    return self->_putingCb(stream);
}

unsigned int CMediaReceiverAAC::_putingCb(void *stream) {
    // put raw frame
    audio_raw_frame_t *frame = reinterpret_cast<audio_raw_frame_t *>(stream);
    return m_fifowriter->PutAppending(frame->ptr, frame->len);
}

int CMediaReceiverAAC::RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags) {
    ZC_ASSERT(m_fifowriter != nullptr);
    struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC, &_ts);
    if (bytes + 7 <= ZC_STREAM_MAXFRAME_SIZE_A) {
        _framelenUpdateADTSHdr(bytes);
        memcpy(m_frame->data, m_dtshdr, 7);

        m_frame->keyflag = 0;
        m_frame->seq = 0;
        m_frame->utc = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
        m_frame->pts = m_frame->utc;
        m_frame->size = bytes + 7;
        audio_raw_frame_t raw;
        raw.ptr = (const uint8_t *)packet;
        raw.len = bytes;

        // put hdr+adts hdr; callback put audio raw data.
        m_fifowriter->Put((const unsigned char *)m_frame, sizeof(zc_frame_t) + 7, &raw);

#if ZC_DEBUG
        LOG_TRACE("AAC,time:%08u,utc:%u,len:%d,flags:%d", time, m_frame->size, m_frame->utc, flags);
#endif
    }

    return 0;
}
}  // namespace zc
