// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <stdio.h>
#include <string.h>

#include "ZcType.hpp"
#include "sys/system.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcMediaReceiverH265.hpp"

namespace zc {
CMediaReceiverH265::CMediaReceiverH265(const zc_meida_track_t &info) : CMediaReceiver(info) {
    m_frame = (zc_frame_t *)m_framebuf;
    memset(m_frame, 0, sizeof(zc_frame_t));
    m_frame->magic = ZC_FRAME_VIDEO_MAGIC;
    m_frame->type = ZC_STREAM_VIDEO;
    m_frame->video.encode = ZC_FRAME_ENC_H265;
    // init package
    memset(&m_spsinfo, 0, sizeof(m_spsinfo));
    m_pkgcnt = 0;
    m_lasttime = 0;
    LOG_TRACE("debug framemaxlen:%u", m_info.framemaxlen);
}

CMediaReceiverH265::~CMediaReceiverH265() {}

bool CMediaReceiverH265::Init(void *pinfo) {
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
    LOG_TRACE("Create ok ,H265");
    return true;
_err:
    LOG_ERROR("Create error H265");
    ZC_SAFE_DELETE(m_fifowriter);
    return false;
}

unsigned int CMediaReceiverH265::putingCb(void *u, void *stream) {
    CMediaReceiverH265 *self = reinterpret_cast<CMediaReceiverH265 *>(u);
    return self->_putingCb(stream);
}

unsigned int CMediaReceiverH265::_putingCb(void *stream) {
    // TODO:
    // return m_fifowriter->PutAppending();
    return 0;
}

int CMediaReceiverH265::RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags) {
    // int ret = 0;
    // zc_frame_t *pframe = (zc_frame_t *)m_framebuf;
    // ret = m_fifowriter->Put(m_framebuf, sizeof(m_framebuf), sizeof(zc_frame_t), ZC_FRAME_VIDEO_MAGIC);
    // if (pframe->keyflag) {
    //     struct timespec _ts;
    //     clock_gettime(CLOCK_MONOTONIC, &_ts);
    //     unsigned int now = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
    //     LOG_TRACE("rtsp:pts:%u,utc:%u,now:%u,len:%d,cos:%dms", pframe->pts, pframe->utc, now, pframe->size,
    //               now - pframe->utc);
    // }

    uint8_t type = *(uint8_t *)packet & 0x1f;
    struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC, &_ts);
    m_lasttime = _ts.tv_sec;

    if (type >= H265_NAL_UNIT_CODED_SLICE_TRAIL_N && type <= H265_NAL_UNIT_CODED_SLICE_RASL_R) {
        // TODO(zhoucc): B frame
    } else if (type >= H265_NAL_UNIT_CODED_SLICE_BLA_W_LP && type <= H265_NAL_UNIT_CODED_SLICE_CRA) {
        // I Frame 16-21,
        // TODO(zhoucc): I frame
    } else if (type >= H265_NAL_UNIT_VPS) {
        // !vcl;
        // LOG_WARN("%p => encoding:H265, time:%08u, flags:%d, bytes:%d", this, time, flags, bytes);
    }

    if (type == H265_NAL_UNIT_SPS) {
        zc_h26x_sps_info_t spsinfo = {0};
        if (zc_h265_sps_parse((const uint8_t *)packet, bytes, &spsinfo) == 0) {
            if (memcmp(&m_spsinfo, &spsinfo, sizeof(zc_h26x_sps_info_t)) != 0) {
                LOG_WARN("update h265 wh[%u*%u]->[%u*%u]", spsinfo.width, spsinfo.height, m_spsinfo.width,
                         m_spsinfo.height);
                memcpy(&m_spsinfo, &spsinfo, sizeof(zc_h26x_sps_info_t));
                m_frame->video.width = spsinfo.width;
                m_frame->video.height = spsinfo.height;
            }
        } else {
            LOG_ERROR("h265 parse sps error");
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
        LOG_ERROR("pack error, reset time:%08u, bytes:%u, size:%u,framemaxlen:%u, pkgcnt:%u", time, bytes,
                  m_frame->size, m_info.framemaxlen, m_pkgcnt);
        m_frame->size = 0;
        m_pkgcnt = 0;
        return 0;
    }

    if (type >= 0 && type <= H265_NAL_UNIT_CODED_SLICE_CRA) {
        m_frame->keyflag = (type >= H265_NAL_UNIT_CODED_SLICE_BLA_W_LP) ? ZC_FRAME_IDR : 0;
        m_frame->seq = m_frame->seq + 1;
        m_frame->utc = _ts.tv_sec * 1000 + _ts.tv_nsec / 1000000;
        m_frame->pts = m_frame->utc;

        m_fifowriter->Put((const unsigned char *)m_frame, sizeof(zc_frame_t) + m_frame->size, NULL);

        // if (m_frame->keyflag)
            LOG_TRACE("H265,time:%08u,utc:%u,len:%u,type:%d,flags:%d,wh:%hu*%hu,pkgcnt:%d", time, m_frame->utc,
                      m_frame->size, type, flags, m_frame->video.width, m_frame->video.height, m_pkgcnt);

        m_frame->size = 0;
        m_pkgcnt = 0;
    }

    return 0;
}
}  // namespace zc
