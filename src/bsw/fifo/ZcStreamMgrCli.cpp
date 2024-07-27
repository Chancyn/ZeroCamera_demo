// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// shm fifo

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Thread.hpp"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "ZcShmStream.hpp"
#include "ZcStreamMgrCli.hpp"
#include "ZcType.hpp"

#ifdef ZC_DEBUG
#define ZC_DEBUG_DUMP 1
#endif

namespace zc {

CStreamMgrCli::CStreamMgrCli() : m_init(false), m_running(0) {
    memset(&m_info, 0, sizeof(m_info));
}

CStreamMgrCli::~CStreamMgrCli() {
    UnInit();
}

bool CStreamMgrCli::Init(zc_streamcli_t *info) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    if (!info) {
        return false;
    }

    memcpy(&m_info, &info, sizeof(m_info));
    m_init = true;
    LOG_TRACE("Init ok [%s]pid:%d", m_info.pname, m_info.pid);
    return true;

_err:
    _unInit();

    LOG_TRACE("Init error");
    return false;
}

bool CStreamMgrCli::_unInit() {
    Stop();
    return true;
}

bool CStreamMgrCli::UnInit() {
    if (!m_init) {
        return false;
    }

    _unInit();

    m_init = false;
    return true;
}

int CStreamMgrCli::connectSvr() {
    if (m_running) {
        return -1;
    }
    // send msg to svr

    return 0;
}

int CStreamMgrCli::GetShmStreamInfo(zc_stream_info_t *info, zc_shmstream_e type, unsigned int nchn) {
    if (m_running) {
        return -1;
    }
    // send msg to svr

    return 0;
}

bool CStreamMgrCli::Start() {
    if (m_running) {
        return false;
    }

    Thread::Start();
    m_running = true;
    return true;
}

bool CStreamMgrCli::Stop() {
    if (!m_running) {
        return false;
    }

    Thread::Stop();
    m_running = false;
    return true;
}

int CStreamMgrCli::process() {
    LOG_WARN("process into");
    int ret = 0;

    while (State() == Running) {
        // TODO(zhoucc): do something
        usleep(10 * 1000);
    }
_err:
    LOG_WARN("process exit");
    return ret;
}
}  // namespace zc
