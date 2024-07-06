// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// client
#include <netdb.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/types.h>

#include "cpm/unuse.h"
#include "cstringext.h"
#include "flv-proto.h"
#include "flv-reader.h"
#include "ntp-time.h"
#include "rtmp-client.h"
#include "sockpair.h"
#include "sockutil.h"
#include "sys/system.h"
#include "uri-parse.h"
#include "urlcodec.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcRtmpPush.hpp"
#include "ZcType.hpp"
#include "zc_h26x_sps_parse.h"

#define ZC_RTMP_CLI_BUF_SIZE (2 * 1024 * 1024)

namespace zc {
CRtmpPush::CRtmpPush()
    : Thread("Rtmppush"), m_init(false), m_running(0), m_pbuf(new char[ZC_RTMP_CLI_BUF_SIZE]), m_phandle(nullptr) {
    memset(&m_client, 0, sizeof(m_client));
}

CRtmpPush::~CRtmpPush() {
    UnInit();
    ZC_SAFE_DELETEA(m_pbuf);
}

int CRtmpPush::rtmp_client_send(void *ptr, const void *header, size_t len, const void *data, size_t bytes) {
    CRtmpPush *pcli = reinterpret_cast<CRtmpPush *>(ptr);
    return pcli->_rtmp_client_send(header, len, data, bytes);
}

int CRtmpPush::_rtmp_client_send(const void *header, size_t len, const void *data, size_t bytes) {
    socket_bufvec_t vec[2];
    socket_setbufvec(vec, 0, (void *)header, len);
    socket_setbufvec(vec, 1, (void *)data, bytes);

    return socket_send_v_all_by_time(m_client.socket, vec, bytes > 0 ? 2 : 1, 0, 5000);
}

bool CRtmpPush::Init(const zc_stream_info_t &info, const char *url) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }

    strncpy(m_url, url, sizeof(m_url));
    memcpy(&m_info, &info, sizeof(zc_stream_info_t));
    m_init = true;
    return true;
}

bool CRtmpPush::UnInit() {
    StopCli();
    m_init = false;
    return true;
}

// rtmp://video-center.alivecdn.com/live/hello?vhost=your.domain
// rtmp_publish_test("video-center.alivecdn.com", "live", "hello?vhost=your.domain", local-flv-file-name)
bool CRtmpPush::_startconn() {
    char rurl[256] = {0};  // rtmp url
    char host[128] = {0};
    char path[128] = {0};
    char app[128] = {0};
    char stream[128] = {0};
    char *pstream = nullptr;
    unsigned short port = 1593;
    int r = 0;
    strncpy(rurl, m_url, sizeof(rurl) - 1);
    struct rtmp_client_handler_t *phandle = nullptr;
    struct rtmp_client_t *rtmp = nullptr;
    // parse url
    struct uri_t *url = uri_parse(rurl, strlen(rurl));
    if (!url)
        return false;

    // prase port
    url_decode(url->path, strlen(url->path), path, sizeof(path));
    url_decode(url->host, strlen(url->host), host, sizeof(host));
    strncpy(m_host, host, sizeof(host) - 1);
    pstream = strrchr(url->path, '/');
    if (!pstream) {
        LOG_ERROR("rtmppush prase error url:%s path:%s", m_url, url->path);
        uri_free(url);
        return false;
    }

    *pstream = '\0';
    strncpy(app, url->path + 1, sizeof(app) - 1);
    strncpy(stream, pstream + 1, sizeof(stream) - 1);
    if (app[0] == '\0' || stream[0] == '\0') {
        LOG_ERROR("rtmppush prase error url:%s path:%s", m_url, url->path);
        uri_free(url);
        return false;
    }

    if (url->port != 0) {
        port = url->port;
        snprintf(rurl, sizeof(rurl) - 1, "rtmp://%s:%hu/%s", m_host, port, app);
    } else {
        snprintf(rurl, sizeof(rurl) - 1, "rtmp://%s/%s", m_host, app);
    }

    LOG_TRACE("rtmppush url:%s, host:%s, port:%hu, app:%s, stream:%s", rurl, m_host, port, app, stream);
    phandle = (struct rtmp_client_handler_t *)malloc(sizeof(struct rtmp_client_handler_t));
    ZC_ASSERT(phandle);
    if (!phandle) {
        LOG_ERROR("rtmppush malloc error url[%s] this[%p]", m_url, this);
        goto _err;
    }
    memset(phandle, 0, sizeof(rtmp_client_handler_t));
    m_phandle = phandle;
    phandle->send = rtmp_client_send;

    socket_init();
    m_client.socket = socket_connect_host(host, 1935, 2000);
    socket_setnonblock(m_client.socket, 0);

    rtmp = rtmp_client_create(app, stream, rurl, this, phandle);
    if (!rtmp) {
        LOG_ERROR("rtmp rtmp_client_create error");
        ZC_ASSERT(0);
        goto _err;
    }

    m_client.rtmp = rtmp;
    if (rtmp_client_start(rtmp, 0) < 0) {
        LOG_ERROR("rtmp rtmp_client_start error");
        goto _err;
    }

    while (4 != rtmp_client_getstate(m_client.rtmp) &&
           (r = socket_recv(m_client.socket, m_pbuf, ZC_RTMP_CLI_BUF_SIZE, 0)) > 0) {
        if (rtmp_client_input(m_client.rtmp, m_pbuf, r) != 0) {
            LOG_ERROR("rtmp client input error");
            goto _err;
        }
    }
    m_client.status = 1;
    uri_free(url);
    LOG_TRACE("rtmppush starcomm ok");
    return true;
_err:
    _stopconn();
    uri_free(url);
    m_client.status = 0;
    LOG_ERROR("rtmppush _startconn error");
    return false;
}

bool CRtmpPush::StartCli() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CRtmpPush::StopCli() {
    LOG_TRACE("Stop into");
    if (m_running) {
        // first shutdown socket wakeup block recv thead
        if (m_client.socket > 0)
            socket_shutdown(m_client.socket, SHUT_RDWR);
        LOG_TRACE("socket_shutdown socket", m_client.socket);
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CRtmpPush::_stopconn() {
    if (m_client.rtmp) {
        rtmp_client_destroy(reinterpret_cast<rtmp_client_t *>(m_client.rtmp));
        m_client.rtmp = nullptr;
    }

    if (m_client.socket > 0) {
        socket_close(m_client.socket);
        m_client.socket = 0;
        m_client.status = 0;
        socket_cleanup();
    }

    ZC_SAFE_FREE(m_phandle);
    return true;
}

int CRtmpPush::onFlvPacketCb(void *ptr, int type, const void *data, size_t bytes, uint32_t timestamp) {
    CRtmpPush *pcli = reinterpret_cast<CRtmpPush *>(ptr);
    return pcli->_onFlvPacketCb(type, data, bytes, timestamp);
}

int CRtmpPush::_onFlvPacketCb(int type, const void *data, size_t bytes, uint32_t timestamp) {
    if (!m_client.status) {
        return 0;
    }

    int ret = 0;
    if (FLV_TYPE_AUDIO == type) {
        ret = rtmp_client_push_audio(m_client.rtmp, data, bytes, timestamp);
    } else if (FLV_TYPE_VIDEO == type) {
#if ZC_DEBUG
        int keyframe = 1 == (((*(unsigned char *)data) & 0xF0) >> 4);
        if (keyframe)
            LOG_TRACE("type:%02d [A:%d, V:%d, S:%d] key:%d\n", type, FLV_TYPE_AUDIO, FLV_TYPE_VIDEO, FLV_TYPE_SCRIPT,
                      (type == FLV_TYPE_VIDEO) ? keyframe : 0);
#endif
        ret = rtmp_client_push_video(m_client.rtmp, data, bytes, timestamp);
    } else if (FLV_TYPE_SCRIPT == type) {
        ret = rtmp_client_push_script(m_client.rtmp, data, bytes, timestamp);
    } else {
        ZC_ASSERT(0);
        ret = 0;  // ignore
    }
    return ret;
}

bool CRtmpPush::_startFlvMuxer() {
    m_flv = new CFlvMuxer();
    if (!m_flv) {
        return false;
    }
    zc_flvmuxer_info_t info = {
        .onflvpacketcb = onFlvPacketCb,
        .Context = this,
    };

    memcpy(&info.streaminfo, &m_info, sizeof(zc_stream_info_t));

    if (!m_flv->Create(info)) {
        LOG_ERROR("flvmuxer create error");
        goto _err;
    }

    if (!m_flv->Start()) {
        LOG_ERROR("flvmuxer start error");
        goto _err;
    }

    LOG_TRACE("startFlvMuxer OK");
    return true;
_err:
    m_flv->Destroy();
    delete m_flv;
    m_flv = nullptr;

    LOG_ERROR("startFlvMuxer error");
    return false;
}

bool CRtmpPush::_stopFlvMuxer() {
    if (m_flv) {
        m_flv->Stop();
        m_flv->Destroy();
        delete m_flv;
        m_flv = nullptr;
    }

    return true;
}

int CRtmpPush::_cliwork() {
    // start
    int ret = 0;
    if (!_startconn()) {
        LOG_ERROR("_startconn error");
        return -1;
    }

    if (!_startFlvMuxer()) {
        LOG_ERROR("_startFlvMuxer error");
        goto _err;
    }

    while (State() == Running && m_client.status == 1) {
        system_sleep(5);
        // TODO: check rtsp session activity
    }

_err:
    _stopFlvMuxer();
    // rtsp_client_teardown(reinterpret_cast<rtsp_client_t *>(m_client.rtsp));
    // stop
    _stopconn();
    return ret;
}

int CRtmpPush::process() {
    LOG_WARN("process into\n");
    while (State() == Running /*&&  i < loopcnt*/) {
        if (_cliwork() < 0) {
            break;
        }
        system_sleep(1000);
    }
    LOG_WARN("process exit\n");
    return -1;
}

}  // namespace zc
