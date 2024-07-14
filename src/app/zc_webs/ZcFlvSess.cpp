// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "Thread.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg_sys.h"

#include "mongoose.h"

#include "ZcFlvSess.hpp"
#include "ZcType.hpp"
#include "ZcWebServer.hpp"

namespace zc {

CFlvSess::CFlvSess(zc_flvsess_type_e type, const zc_flvsess_info_t &info)
    : m_type(type), m_status(zc_flvsess_uninit_e), m_bsendhdr(false) {
    memcpy(&m_info, &info, sizeof(zc_flvsess_info_t));
    return;
}

CFlvSess::~CFlvSess() {
    return;
}

bool CFlvSess::Open() {
    if (m_status > zc_flvsess_uninit_e) {
        return false;
    }

    zc_flvmuxer_info_t muxerinfo = {
        .streaminfo = m_info.streaminfo,
        .onflvpacketcb = OnFlvPacketCb,
        .Context = this,
    };

    // TODO(zhoucc): check info
    if (!m_flvmuxer.Create(muxerinfo)) {
        LOG_ERROR("flvmuxer create error");
        m_status = zc_flvsess_err_e;
        return false;
    }

    m_status = zc_flvsess_init_e;
    LOG_TRACE("Session Start ok");
    return true;
}

bool CFlvSess::StartSendProcess() {
    if (m_status != zc_flvsess_init_e) {
        return false;
    }

    if (!m_flvmuxer.Start()) {
        LOG_ERROR("flvmuxer start error");
        m_status = zc_flvsess_err_e;
        m_flvmuxer.Destroy();
        return false;
    }
    m_status = zc_flvsess_sending_e;

    return true;
}

bool CFlvSess::Close() {
    if (m_status <= zc_flvsess_uninit_e) {
        return true;
    }

    m_flvmuxer.Stop();
    m_flvmuxer.Destroy();
    m_status = zc_flvsess_uninit_e;
    return true;
}

int CFlvSess::OnFlvPacketCb(void *param, int type, const void *data, size_t bytes, uint32_t timestamp) {
    CFlvSess *sess = reinterpret_cast<CFlvSess *>(param);
    return sess->_onFlvPacketCb(type, data, bytes, timestamp);
}

int CFlvSess::_onFlvPacketCb(int type, const void *data, size_t bytes, uint32_t timestamp) {
    if (!m_status) {
        return 0;
    }

    int ret = 0;
    // send hdr
    if (!m_bsendhdr) {
        ret = m_info.sendflvhdrcb(m_info.context, m_info.connsess, true, false);
        m_bsendhdr = true;
    }
#if 1
    ret = m_info.sendflvdatacb(m_info.context, m_info.connsess, type, data, bytes, timestamp);
#else
    if (flv_tag_audio == type) {
        // ret = rtmp_client_push_audio(m_client.rtmp, data, bytes, timestamp);
        ret = m_info.sendflvdatacb(m_info.context, m_info.connsess, type, data, bytes, timestamp);
    } else if (flv_tag_video == type) {
#if ZC_DEBUG
        int keyframe = 1 == (((*(unsigned char *)data) & 0xF0) >> 4);
        if (keyframe)
            LOG_TRACE("type:%02d [A:%d, V:%d, S:%d] key:%d", type, flv_tag_audio, flv_tag_video, flv_tag_script,
                      (type == flv_tag_video) ? keyframe : 0);
#endif
        // ret = rtmp_client_push_video(m_client.rtmp, data, bytes, timestamp);
        ret = m_info.sendflvdatacb(m_info.context, m_info.connsess, type, data, bytes, timestamp);
    } else if (flv_tag_script == type) {
        // ret = rtmp_client_push_script(m_client.rtmp, data, bytes, timestamp);
        ret = m_info.sendflvdatacb(m_info.context, m_info.connsess, type, data, bytes, timestamp);
    } else {
        ZC_ASSERT(0);
        ret = 0;  // ignore
    }
#endif
    return ret;
}

#if ZC_SUPPORT_HTTP_FLV

#endif

}  // namespace zc
