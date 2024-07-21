// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>

#include "zc_log.h"
#include "zc_msg_sys.h"

#include "ZcSysManager.hpp"
#include "ZcType.hpp"

namespace zc {
CSysManager::CSysManager() : m_init(false), m_running(0) {}

CSysManager::~CSysManager() {
    _unInit();
}

bool CSysManager::Init(sys_callback_info_t *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    if (!CModSys::Init(cbinfo)) {
        LOG_TRACE("CModRtsp Init error");
        goto _err;
    }

    m_init = true;
    LOG_TRACE("Init ok");
    return true;

_err:
    unInit();

    LOG_TRACE("Init error");
    return false;
}

bool CSysManager::_unInit() {
    if (!m_init) {
        return false;
    }
    Stop();
    CModSys::UnInit();
    return true;
}

bool CSysManager::UnInit() {
    if (!m_init) {
        return false;
    }

    _unInit();

    m_init = false;
    return true;
}

bool CSysManager::Start() {
    if (m_running) {
        return false;
    }

    CModSys::Start();

    m_running = true;
    return true;
}

bool CSysManager::Stop() {
    if (!m_running) {
        return false;
    }

    CModSys::Stop();

    m_running = false;
    return true;
}

int CSysManager::PublishMsgCb(void *ptr, ZC_U16 smid, ZC_U16 smsid, void *msg, unsigned int len) {
    CSysManager *pSys = reinterpret_cast<CSysManager *>(ptr);
    return pSys->PublishMsg(smid, smsid, msg, len);
}

int CSysManager::PublishMsg(ZC_U16 smid, ZC_U16 smsid, void *msg, unsigned int len) {
    if (!msg) {
        len = 0;
    }

    LOG_INFO("pub ,smid:%u,smsid:%u, len:%u", smid, smsid, len);
    unsigned int msglen = sizeof(zc_msg_t) + len;
    char *msg_buf = new char[msglen]();
    if (!msg_buf) {
        return -1;
    }
    zc_msg_t *sub = reinterpret_cast<zc_msg_t *>(msg_buf);
    BuildPubMsgHdr(sub, smid, smsid, 0, len);
    if (len > 0)
        memcpy(sub->data, msg, len);

    if (!Publish(sub, msglen)) {
        LOG_ERROR("Publish err:%d \n");
    }
    delete[] msg_buf;
    LOG_TRACE("PublishMsg ok");
    return 0;
}

}  // namespace zc
