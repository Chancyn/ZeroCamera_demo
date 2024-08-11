// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcSrtPush.hpp"
#include "ZcTsMuxer.hpp"
#include "ZcType.hpp"
#include "zc_h26x_sps_parse.h"

#define ZC_SRT_CLI_BUF_SIZE (2 * 1024 * 1024)

namespace zc {
CSrtPush::CSrtPush()
    : Thread("srtpush"), m_init(false), m_running(0), m_pbuf(new char[ZC_SRT_CLI_BUF_SIZE]), m_phandle(nullptr) {
    memset(&m_client, 0, sizeof(m_client));
}

CSrtPush::~CSrtPush() {
    UnInit();
    ZC_SAFE_DELETEA(m_pbuf);
}

bool CSrtPush::Init(const zc_stream_info_t &info, const char *url) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }

    strncpy(m_url, url, sizeof(m_url));
    memcpy(&m_info, &info, sizeof(zc_stream_info_t));
    m_init = true;
    return true;
}

bool CSrtPush::UnInit() {
    StopCli();
    m_init = false;
    return true;
}

// srt://video-center.alivecdn.com/live/hello?vhost=your.domain
// srt_publish_test("video-center.alivecdn.com", "live", "hello?vhost=your.domain", local-MpegTs-file-name)
bool CSrtPush::_startconn() {
    char rurl[256] = {0};  // srt url
    char host[128] = {0};
    char path[128] = {0};
    char app[128] = {0};
    char stream[128] = {0};
    char *pstream = nullptr;
    unsigned short port = 1593;
    int r = 0;
    strncpy(rurl, m_url, sizeof(rurl) - 1);
    struct srt_client_handler_t *phandle = nullptr;
    struct srt_client_t *srt = nullptr;
    // prase port

    LOG_TRACE("srtpush url:%s, host:%s, port:%hu, app:%s, stream:%s", rurl, m_host, port, app, stream);
    // ZC_ASSERT(phandle);
    // if (!phandle) {
    //     LOG_ERROR("srtpush malloc error url[%s] this[%p]", m_url, this);
    //     goto _err;
    // }
    m_phandle = phandle;

    // srt = srt_client_create(app, stream, rurl, this, phandle);
    // if (!srt) {
    //     LOG_ERROR("srt srt_client_create error");
    //     ZC_ASSERT(0);
    //     goto _err;
    // }

    m_client.srt = srt;
    m_client.status = 1;
    LOG_TRACE("srtpush starcomm ok");
    return true;
_err:
    _stopconn();
    m_client.status = 0;
    LOG_ERROR("srtpush _startconn error");
    return false;
}

bool CSrtPush::StartCli() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CSrtPush::StopCli() {
    LOG_TRACE("Stop into");
    if (m_running) {
        LOG_TRACE("socket_shutdown socket");
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CSrtPush::_stopconn() {
    if (m_client.srt) {
        // srt_client_destroy(reinterpret_cast<srt_client_t *>(m_client.srt));
        m_client.srt = nullptr;
    }

    ZC_SAFE_FREE(m_phandle);
    return true;
}

int CSrtPush::onMpegTsPacketCb(void *ptr, const void *data, size_t bytes) {
    CSrtPush *pcli = reinterpret_cast<CSrtPush *>(ptr);
    return pcli->_onMpegTsPacketCb(data, bytes);
}

int CSrtPush::_onMpegTsPacketCb(const void *data, size_t bytes) {
    if (!m_client.status) {
        return 0;
    }
    // LOG_TRACE("ts onpkg, size:%d", bytes);
    int ret = 0;

    return ret;
}

bool CSrtPush::_startMpegTsMuxer() {
    m_mpegts = new CTsMuxer();
    if (!m_mpegts) {
        return false;
    }
    zc_tsmuxer_info_t info = {};
    info.onTsPacketCb = onMpegTsPacketCb;
    info.Context = this;
    memcpy(&info.streaminfo, &m_info, sizeof(zc_stream_info_t));

    if (!m_mpegts->Create(info)) {
        LOG_ERROR("TsMuxer create error");
        goto _err;
    }

    if (!m_mpegts->Start()) {
        LOG_ERROR("TsMuxer start error");
        goto _err;
    }

    LOG_TRACE("startMpegTsMuxer OK");
    return true;
_err:
    m_mpegts->Destroy();
    delete m_mpegts;
    m_mpegts = nullptr;

    LOG_ERROR("startMpegTsMuxer error");
    return false;
}

bool CSrtPush::_stopMpegTsMuxer() {
    if (m_mpegts) {
        m_mpegts->Stop();
        m_mpegts->Destroy();
        delete m_mpegts;
        m_mpegts = nullptr;
    }

    return true;
}

int CSrtPush::_cliwork() {
    // start
    int ret = 0;
    if (!_startconn()) {
        LOG_ERROR("_startconn error");
        return -1;
    }

    if (!_startMpegTsMuxer()) {
        LOG_ERROR("_startMpegTsMuxer error");
        goto _err;
    }

    while (State() == Running && m_client.status == 1) {
        ZC_MSLEEP(5);
    }

_err:
    _stopMpegTsMuxer();
    // stop
    _stopconn();
    return ret;
}

int CSrtPush::process() {
    LOG_WARN("process into");
    while (State() == Running /*&&  i < loopcnt*/) {
        if (_cliwork() < 0) {
            break;
        }
        ZC_MSLEEP(1000);
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
