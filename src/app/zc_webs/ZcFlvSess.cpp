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

CFlvSess::CFlvSess(zc_web_msess_type_e type, const zc_flvsess_info_t &info)
    : IWebMSess(type), m_status(zc_msess_uninit_e) {
    memcpy(&m_info, &info, sizeof(zc_flvsess_info_t));
    return;
}

CFlvSess::~CFlvSess() {
    return;
}

bool CFlvSess::Open() {
    if (m_status > zc_msess_uninit_e) {
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
        m_status = zc_msess_err_e;
        return false;
    }

    m_status = zc_msess_init_e;
    LOG_TRACE("Session Start ok");
    return true;
}

bool CFlvSess::StartSendProcess() {
    if (m_status != zc_msess_init_e) {
        return false;
    }

    if (!m_flvmuxer.Start()) {
        LOG_ERROR("flvmuxer start error");
        m_status = zc_msess_err_e;
        m_flvmuxer.Destroy();
        return false;
    }
    m_status = zc_msess_sending_e;

    return true;
}

bool CFlvSess::Close() {
    if (m_status <= zc_msess_uninit_e) {
        return true;
    }

    m_flvmuxer.Stop();
    m_flvmuxer.Destroy();
    m_status = zc_msess_uninit_e;
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

    return m_info.sendflvdatacb(m_info.context, m_info.connsess, m_type, type, data, bytes, timestamp);
}
}  // namespace zc
