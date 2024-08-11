// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// svr
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <list>
#include <map>
#include <stdio.h>
#include <string.h>
#include <string>

#include "zc_log.h"
#include "zc_macros.h"

#include "ZcSrtSvr.hpp"

namespace zc {
#if 0
CSrtSvr::CSrtSvr()
    : Thread("srtsvr"), m_init(false), m_running(0), m_status(0)
      m_port(ZC_SRT_PORT), m_phandle(nullptr), m_srtsvr(nullptr) {
    strncpy(m_host, "0.0.0.0", sizeof(m_host));
}

CSrtSvr::~CSrtSvr() {
    UnInit();
}

bool CSrtSvr::Init(ZC_U16 port) {
    if (m_init) {
        return false;
    }

    m_port = port;
    m_init = true;
    return true;
}

bool CSrtSvr::UnInit() {
    Stop();
    m_init = false;
    return true;
}
bool CSrtSvr::Start() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CSrtSvr::Stop() {
    LOG_TRACE("Stop into");
    if (m_running) {
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CSrtSvr::_startsvr() {
    int ret = 0;

    LOG_TRACE("srtsvr, host:%s, port:%hu", m_host, m_port);

    if (!phandle) {
        LOG_ERROR("srtsvr malloc error m_port[%s] this[%p]", m_port, this);
        goto _err;
    }

    m_status = 1;
    LOG_TRACE("_startsvr ok");
    return true;
_err:
    _stopsvr();
    m_status = 0;
    LOG_ERROR("srtsvr _startconn error");
    return false;
}

bool CSrtSvr::_stopsvr() {
    LOG_TRACE("_stopsvr into");
    if (m_srtsvr) {
        ZC_SAFE_FREE(m_phandle);
    }

    LOG_TRACE("_stopsvr ok");
    return true;
}

int CSrtSvr::_svrwork() {
    // start
    int ret = 0;
    if (!_startsvr()) {
        LOG_ERROR("_startconn error");
        return -1;
    }

    while (State() == Running && m_status == 1) {
        ZC_MSLEEP(5);
    }
    LOG_TRACE("_svrwork loop end");
_err:
    // stop
    _stopsvr();
    LOG_ERROR("_svrwork exit");
    return ret;
}

int CSrtSvr::process() {
    LOG_WARN("process into");
    while (State() == Running /*&&  i < loopcnt*/) {
        if (_svrwork() < 0) {
            break;
        }
        ZC_MSLEEP(1000);
    }
    LOG_WARN("process exit");
    return -1;
}
#endif
}  // namespace zc
