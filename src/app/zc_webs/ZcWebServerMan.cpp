// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "ZcModCli.hpp"
#include "ZcWebServer.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcType.hpp"
#include "ZcWebServerMan.hpp"

namespace zc {
// modsyscli
CWebServerMan::CWebServerMan() : CModCli(ZC_MODID_SYSCLI_E), m_init(false), m_running(0) {
    m_supbitmask = ZC_WEBS_BITMASK_DEF;

    for (unsigned int i = 0; i < _SIZEOFTAB(m_webstab); i++) {
        m_webstab[i] = nullptr;
    }
}

CWebServerMan::~CWebServerMan() {
    UnInit();
}

bool CWebServerMan::Init(void *info) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // zc_stream_info_t info;
    // if (sendSMgrGetInfo(0, 0, &info) < 0) {
    //     LOG_TRACE("sendSMgrGetInfo error");
    //     goto _err;
    // }

    // if (m_supbitmask == 0) {
    //     m_supbitmask = 1;  // atleast support http
    // }

    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:

    LOG_TRACE("Init error");
    return false;
}  // namespace zc

bool CWebServerMan::UnInit() {
    if (!m_init) {
        return false;
    }
    Stop();
    m_init = false;
    return true;
}

bool CWebServerMan::Start() {
    if (m_running) {
        return false;
    }

    websvr_cb_info_t cbinfo = {
        .getStreamInfoCb = getStreamInfoCb,
        .MgrContext = this,
    };

    for (unsigned int i = 0; i < _SIZEOFTAB(m_webstab); i++) {
        if (m_supbitmask & (0x1 << i)) {
            ZC_SAFE_DELETE(m_webstab[i]);
            m_webstab[i] = CWebServerFac::CreateWebServer((zc_webs_type_e)i, cbinfo);
            m_webstab[i]->Init();
            m_webstab[i]->Start();
        }
    }

    m_running = true;
    return true;
}

bool CWebServerMan::Stop() {
    if (!m_running) {
        return false;
    }

    for (unsigned int i = 0; i < _SIZEOFTAB(m_webstab); i++) {
#if 1

        ZC_SAFE_DELETE(m_webstab[i]);
#else
        if (m_webstab[i]) {
            m_webstab[i]->Stop();
            m_webstab[i]->UnInit();
            delete m_webstab[i];
            m_webstab[i] = nullptr;
        }
#endif
    }

    m_running = false;
    return true;
}

int CWebServerMan::_getStreamInfoCb(unsigned int type, unsigned int chn, zc_stream_info_t *info) {
    LOG_TRACE("get info type:%u, chn:%d", type, chn);
    // TODO(zhoucc): cb
    return sendSMgrGetInfo(type, chn, info);
}

int CWebServerMan::getStreamInfoCb(void *ptr, unsigned int type, unsigned int chn, zc_stream_info_t *info) {
    CWebServerMan *man = reinterpret_cast<CWebServerMan *>(ptr);
    return man->_getStreamInfoCb(type, chn, info);
}
}  // namespace zc
