// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <stdio.h>
#include <string.h>

#include "sys/system.h"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "ZcMediaReceiverH265.hpp"
#include "ZcType.hpp"

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
    // init pts
    m_frequency = 90000;
    m_timestamp = 0;
    m_pts = INT64_MIN;
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

int CMediaReceiverH265::SetRtpInfo_Rtptime(uint16_t seq, uint32_t timestamp, uint64_t npt) {
    m_lasttime = timestamp;
    m_timestamp = timestamp;
    m_basepts = npt;
    LOG_WARN("H265,seq:%u timestamp:%u, npt:%llu, %llu", seq, timestamp, npt, m_basepts);
  
    return 0;
}

int CMediaReceiverH265::RtpOnFrameIn(const void *packet, int bytes, uint32_t time, int flags) {
    uint8_t type = ((*(uint8_t *)packet) & 0x7F) >> 1;

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
                LOG_WARN("update h265 wh[%u*%u]->[%u*%u]", m_spsinfo.width, m_spsinfo.height, spsinfo.width,
                         spsinfo.height);
                memcpy(&m_spsinfo, &spsinfo, sizeof(zc_h26x_sps_info_t));
                m_frame->video.width = spsinfo.width;
                m_frame->video.height = spsinfo.height;
                // m_frame->video.keyflag = 1;
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
        // RTP timestamp => PTS/DTS
        if (0 == m_lasttime && INT64_MIN == m_pts) {
            m_timestamp = time;
            m_basepts = 0; // zc_system_time();
            LOG_ERROR("error init timestamp:%lld, pts:%llu", m_timestamp, m_basepts);
        } else {
            // m_timestamp += (int32_t)(time - m_lasttime);
        }
        m_lasttime = time;
        m_pts = m_basepts + (time - m_timestamp) * 1000 / m_frequency;
        
        m_frame->keyflag = (type >= H265_NAL_UNIT_CODED_SLICE_BLA_W_LP) ? ZC_FRAME_IDR : 0;
        m_frame->seq = m_frame->seq + 1;
        m_frame->utc = zc_system_time();
        m_frame->pts = m_pts;    // m_frame->utc;

        m_fifowriter->Put((const unsigned char *)m_frame, sizeof(zc_frame_t) + m_frame->size, NULL);

        if (m_frame->keyflag)
            LOG_TRACE("H265,timestamp:%u, time:%u, pts:%u, utc:%u,len:%u,type:%d,flags:%d,wh:%hu*%hu,pkgcnt:%d", m_timestamp, time, m_frame->pts, m_frame->utc,
                      m_frame->size, type, flags, m_frame->video.width, m_frame->video.height, m_pkgcnt);

        m_frame->size = 0;
        m_pkgcnt = 0;
    }

    return 0;
}
}  // namespace zc
