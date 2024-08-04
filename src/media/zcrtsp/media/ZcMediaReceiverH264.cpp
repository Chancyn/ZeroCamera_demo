// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <stdio.h>
#include <string.h>

#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcMediaReceiverH264.hpp"
#include "ZcType.hpp"

namespace zc {
CMediaReceiverH264::CMediaReceiverH264(const zc_meida_track_t &info) : CMediaReceiver(info) {
    m_frame = (zc_frame_t *)m_framebuf;
    memset(m_frame, 0, sizeof(zc_frame_t));
    m_frame->magic = ZC_FRAME_VIDEO_MAGIC;
    m_frame->type = ZC_STREAM_VIDEO;
    m_frame->video.encode = ZC_FRAME_ENC_H264;
    // init package
    memset(&m_spsinfo, 0, sizeof(m_spsinfo));
    m_pkgcnt = 0;
    LOG_TRACE("Create Constructor [%p]", m_frame);
}

CMediaReceiverH264::~CMediaReceiverH264() {}

bool CMediaReceiverH264::Init(void *pinfo) {
    m_fifowriter = new CShmStreamW(m_info.fifosize, m_info.name, m_info.chn, putingCb, this);
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
    LOG_TRACE("Create ok ,H264");
    return true;
_err:
    LOG_ERROR("Create error H264");
    ZC_SAFE_DELETE(m_fifowriter);
    return false;
}

unsigned int CMediaReceiverH264::putingCb(void *u, void *stream) {
    CMediaReceiverH264 *self = reinterpret_cast<CMediaReceiverH264 *>(u);
    return self->_putingCb(stream);
}

unsigned int CMediaReceiverH264::_putingCb(void *stream) {
    // TODO:
    // return m_fifowriter->PutAppending();
    return 0;
}

int CMediaReceiverH264::RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags) {
    uint8_t type = *(uint8_t *)packet & 0x1f;

    // sps;
    if (type == H264_NAL_UNIT_TYPE_SPS) {
        zc_h26x_sps_info_t spsinfo = {0};
        if (zc_h264_sps_parse((const uint8_t *)packet, bytes, &spsinfo) == 0) {
            if (memcmp(&m_spsinfo, &spsinfo, sizeof(zc_h26x_sps_info_t)) != 0) {
                LOG_WARN("update h264 wh[%u*%u]->[%u*%u]", m_spsinfo.width, m_spsinfo.height, spsinfo.width,
                         spsinfo.height);
                memcpy(&m_spsinfo, &spsinfo, sizeof(zc_h26x_sps_info_t));
                m_frame->video.width = spsinfo.width;
                m_frame->video.height = spsinfo.height;
            }
        } else {
            LOG_ERROR("h264 parse sps error");
        }
    }

    if (m_frame->size + 4 + bytes <= m_info.framemaxlen) {
        m_frame->data[m_frame->size + 0] = 00;
        m_frame->data[m_frame->size + 1] = 00;
        m_frame->data[m_frame->size + 2] = 00;
        m_frame->data[m_frame->size + 3] = 01;

        memcpy(m_frame->data + m_frame->size + 4, packet, bytes);
        m_frame->video.nalu[m_pkgcnt] = bytes + 4;
        m_frame->size += bytes + 4;
        m_pkgcnt++;
    } else {
        LOG_ERROR("pack error, reset time:%08u, bytes[%u], size[%u], pkgcnt[%u]", time, bytes, m_frame->size, m_pkgcnt);
        m_frame->size = 0;
        m_pkgcnt = 0;
        return 0;
    }

    // I frame, P Frame
    if (type == H264_NAL_UNIT_TYPE_CODED_SLICE_IDR || type == H264_NAL_UNIT_TYPE_CODED_SLICE_NON_IDR) {
        m_frame->keyflag = (type == H264_NAL_UNIT_TYPE_CODED_SLICE_IDR) ? ZC_FRAME_IDR : 0;
        m_frame->seq = m_frame->seq + 1;
        m_frame->utc = zc_system_time();
        m_frame->pts = m_frame->utc;

        m_fifowriter->Put((const unsigned char *)m_frame, sizeof(zc_frame_t) + m_frame->size, NULL);

        if (m_frame->keyflag)
            LOG_TRACE("H264,time:%08u,utc:%u,len:%u,type:%d,flags:%d,wh:%hu*%hu", time, m_frame->utc, m_frame->size,
                      type, flags, m_frame->video.width, m_frame->video.height);
        // putto fifo

        // set bufpos
        m_frame->size = 0;
        m_pkgcnt = 0;
    }

    return 0;
}
}  // namespace zc
