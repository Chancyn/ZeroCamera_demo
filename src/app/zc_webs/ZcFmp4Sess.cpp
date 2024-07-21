// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "Thread.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg_sys.h"

#include "mongoose.h"

#include "ZcFmp4Sess.hpp"
#include "ZcType.hpp"
#include "ZcWebServer.hpp"

namespace zc {

CFmp4Sess::CFmp4Sess(zc_fmp4sess_type_e type, const zc_fmp4sess_info_t &info)
    : m_type(type), m_status(zc_fmp4sess_uninit_e) {
    memcpy(&m_info, &info, sizeof(zc_fmp4sess_info_t));
    return;
}

CFmp4Sess::~CFmp4Sess() {
    return;
}

bool CFmp4Sess::Open() {
    if (m_status > zc_fmp4sess_uninit_e) {
        return false;
    }

    zc_fmp4muxer_info_t muxerinfo = {
        .type = fmp4_movio_buf,
        .name = nullptr,
        .streaminfo = m_info.streaminfo,
        .onfmp4packetcb = OnFmp4PacketCb,
        .Context = this,
    };

    // TODO(zhoucc): check info
    if (!m_fmp4muxer.Create(muxerinfo)) {
        LOG_ERROR("fmp4muxer create error");
        m_status = zc_fmp4sess_err_e;
        return false;
    }

    m_status = zc_fmp4sess_init_e;
    LOG_TRACE("Session Start ok");
    return true;
}

bool CFmp4Sess::StartSendProcess() {
    if (m_status != zc_fmp4sess_init_e) {
        return false;
    }

    if (!m_fmp4muxer.Start()) {
        LOG_ERROR("fmp4muxer start error");
        m_status = zc_fmp4sess_err_e;
        m_fmp4muxer.Destroy();
        return false;
    }
    m_status = zc_fmp4sess_sending_e;

    return true;
}

bool CFmp4Sess::Close() {
    if (m_status <= zc_fmp4sess_uninit_e) {
        return true;
    }

    m_fmp4muxer.Stop();
    m_fmp4muxer.Destroy();
    m_status = zc_fmp4sess_uninit_e;
    return true;
}

int CFmp4Sess::OnFmp4PacketCb(void *param, int type, const void *data, size_t bytes, uint32_t timestamp) {
    CFmp4Sess *sess = reinterpret_cast<CFmp4Sess *>(param);
    return sess->_onFmp4PacketCb(type, data, bytes, timestamp);
}

int CFmp4Sess::_onFmp4PacketCb(int type, const void *data, size_t bytes, uint32_t timestamp) {
    if (!m_status) {
        return 0;
    }

    int ret = 0;
    ret = m_info.sendfmp4datacb(m_info.context, m_info.connsess, type, data, bytes, timestamp);
    return ret;
}

}  // namespace zc
