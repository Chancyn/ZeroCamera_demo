// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "Thread.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg_sys.h"

#include "mongoose.h"

#include "ZcTsSess.hpp"
#include "ZcType.hpp"
#include "ZcWebServer.hpp"

namespace zc {

CTsSess::CTsSess(zc_web_msess_type_e type, const zc_tssess_info_t &info)
    : IWebMSess(type), m_status(zc_msess_uninit_e) {
    memcpy(&m_info, &info, sizeof(zc_tssess_info_t));
    return;
}

CTsSess::~CTsSess() {
    return;
}

bool CTsSess::Open() {
    if (m_status > zc_msess_uninit_e) {
        return false;
    }

    zc_tsmuxer_info_t muxerinfo = {
        .streaminfo = m_info.streaminfo,
        .onTsPacketCb = OnTsPacketCb,
        .Context = this,
    };

    // TODO(zhoucc): check info
    if (!m_tsmuxer.Create(muxerinfo)) {
        LOG_ERROR("tsmuxer create error");
        m_status = zc_msess_err_e;
        return false;
    }

    m_status = zc_msess_init_e;
    LOG_TRACE("Session Start ok");
    return true;
}

bool CTsSess::StartSendProcess() {
    if (m_status != zc_msess_init_e) {
        return false;
    }

    if (!m_tsmuxer.Start()) {
        LOG_ERROR("tsmuxer start error");
        m_status = zc_msess_err_e;
        m_tsmuxer.Destroy();
        return false;
    }
    m_status = zc_msess_sending_e;

    return true;
}

bool CTsSess::Close() {
    if (m_status <= zc_msess_uninit_e) {
        return true;
    }

    m_tsmuxer.Stop();
    m_tsmuxer.Destroy();
    m_status = zc_msess_uninit_e;
    return true;
}

int CTsSess::OnTsPacketCb(void *param, const void *data, size_t bytes) {
    CTsSess *sess = reinterpret_cast<CTsSess *>(param);
    return sess->_onTsPacketCb(data, bytes);
}

int CTsSess::_onTsPacketCb(const void *data, size_t bytes) {
    if (!m_status) {
        return 0;
    }

    return m_info.sendtsdatacb(m_info.context, m_info.connsess, m_type, data, bytes);
}
}  // namespace zc
