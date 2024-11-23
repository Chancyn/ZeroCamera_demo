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

#include "aio-connect.h"
#include "aio-rtmp-client.h"
#include "aio-timeout.h"
#include "cpm/unuse.h"
#include "cstringext.h"
#include "flv-proto.h"
#include "flv-reader.h"
#include "ntp-time.h"
#include "sockpair.h"
#include "sockutil.h"
#include "sys/system.h"
#include "uri-parse.h"
#include "urlcodec.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcRtmpPushAioCli.hpp"
#include "ZcType.hpp"
#include "zc_h26x_sps_parse.h"
#include "zc_rtmp_utils.h"

// aio buf, pause send
#define ZC_RTMP_CLI_UNSENDBUF_BLOCK (8 * 1024 * 1024)

namespace zc {
CRtmpPushAioCli::CRtmpPushAioCli() : Thread("RtmpPushAio"), m_init(false), m_running(0), m_phandle(nullptr) {
    memset(&m_client, 0, sizeof(m_client));
}

CRtmpPushAioCli::~CRtmpPushAioCli() {
    UnInit();
}

void CRtmpPushAioCli::rtmpClientPublishOnSend(void *ptr, size_t len) {
    CRtmpPushAioCli *pcli = reinterpret_cast<CRtmpPushAioCli *>(ptr);
    return pcli->_rtmpClientPublishOnSend(len);
}

void CRtmpPushAioCli::_rtmpClientPublishOnSend(size_t len) {
    // LOG_TRACE("rtmppush onsend len:%zu", len);
    return;
}

void CRtmpPushAioCli::rtmpClientPublishOnReady(void *ptr) {
    CRtmpPushAioCli *pcli = reinterpret_cast<CRtmpPushAioCli *>(ptr);
    return pcli->_rtmpClientPublishOnReady();
}

void CRtmpPushAioCli::_rtmpClientPublishOnReady() {
    LOG_TRACE("rtmppush onready, _startFlvMuxer");
    m_client.status = 1;
    if (!_startFlvMuxer()) {
        m_client.status = 0;
        LOG_ERROR("_startFlvMuxer error set status %d", m_client.status);
    }

    return;
}

void CRtmpPushAioCli::rtmpClientPublishOnError(void *ptr, int code) {
    CRtmpPushAioCli *pcli = reinterpret_cast<CRtmpPushAioCli *>(ptr);
    return pcli->_rtmpClientPublishOnError(code);
}

void CRtmpPushAioCli::_rtmpClientPublishOnError(int code) {
    LOG_TRACE("rtmppush onerror");
    if (m_client.status) {
        _stopFlvMuxer();
        m_client.status = 0;
        LOG_ERROR("PublishOnError error, _stopFlvMuxer set status %d", m_client.status);
    }

    return;
}

bool CRtmpPushAioCli::Init(const zc_stream_info_t &info, const char *url) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }

    strncpy(m_url, url, sizeof(m_url));
    memcpy(&m_info, &info, sizeof(zc_stream_info_t));
    m_init = true;
    return true;
}

bool CRtmpPushAioCli::UnInit() {
    StopCli();
    m_init = false;
    return true;
}

void CRtmpPushAioCli::aioOnConnect(void *ptr, int code, socket_t tcp, aio_socket_t aio) {
    CRtmpPushAioCli *pcli = reinterpret_cast<CRtmpPushAioCli *>(ptr);
    return pcli->_aioOnConnect(code, tcp, aio);
}

// rtmp://video-center.alivecdn.com/live/hello?vhost=your.domain
// rtmp_publish_test("video-center.alivecdn.com", "live", "hello?vhost=your.domain", local-flv-file-name)
void CRtmpPushAioCli::_aioOnConnect(int code, socket_t tcp, aio_socket_t aio) {
    if (code != 0) {
        m_client.status = -1;
        LOG_TRACE("aioOnConnect error, code:%d", code);
        return;
    }

    struct aio_rtmp_client_handler_t *phandle = nullptr;
    struct aio_rtmp_client_t *rtmp = nullptr;
    LOG_TRACE("rtmppush url:%s, host:%s, port:%hu, app:%s, stream:%s", m_rtmpurl.rurl, m_rtmpurl.host, m_rtmpurl.port,
              m_rtmpurl.app, m_rtmpurl.stream);
    phandle = (struct aio_rtmp_client_handler_t *)malloc(sizeof(struct aio_rtmp_client_handler_t));
    ZC_ASSERT(phandle);
    if (!phandle) {
        LOG_ERROR("rtmppush malloc error url[%s] this[%p]", m_url, this);
        goto _err;
    }
    memset(phandle, 0, sizeof(aio_rtmp_client_handler_t));
    m_phandle = phandle;
    phandle->onerror = rtmpClientPublishOnError;
    phandle->onready = rtmpClientPublishOnReady;
    phandle->onsend = rtmpClientPublishOnSend;

    rtmp = aio_rtmp_client_create(aio, m_rtmpurl.app, m_rtmpurl.stream, m_rtmpurl.rurl, phandle, this);
    if (!rtmp) {
        LOG_ERROR("rtmp aio_rtmp_client_create error");
        ZC_ASSERT(0);
        goto _err;
    }

    m_client.rtmp = rtmp;
    if (aio_rtmp_client_start(rtmp, 0) < 0) {
        LOG_ERROR("rtmp aio_rtmp_client_start error");
        goto _err;
    }

    m_client.status = 1;
    LOG_TRACE("rtmppushaio starcomm ok");
    return;
_err:
    _stopconn();
    m_client.status = 0;
    LOG_ERROR("rtmppushaio _startconn error");
    return;
}

bool CRtmpPushAioCli::StartCli() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    if (!zc_rtmp_prase_url(m_url, &m_rtmpurl)) {
        LOG_ERROR("prase rtmpurl error :%s", m_url);
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CRtmpPushAioCli::StopCli() {
    LOG_TRACE("Stop into");
    if (m_running) {
        LOG_TRACE("set status:%d", m_client.status);
        m_client.status = 0;
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CRtmpPushAioCli::_stopconn() {
    if (m_client.rtmp) {
        aio_rtmp_client_destroy(reinterpret_cast<aio_rtmp_client_t *>(m_client.rtmp));
        m_client.rtmp = nullptr;
    }

    ZC_SAFE_FREE(m_phandle);
    return true;
}

int CRtmpPushAioCli::onFlvPacketCb(void *ptr, int type, const void *data, size_t bytes, uint32_t timestamp) {
    CRtmpPushAioCli *pcli = reinterpret_cast<CRtmpPushAioCli *>(ptr);
    return pcli->_onFlvPacketCb(type, data, bytes, timestamp);
}

int CRtmpPushAioCli::_onFlvPacketCb(int type, const void *data, size_t bytes, uint32_t timestamp) {
    if (m_client.status <= 0) {
        LOG_TRACE("_onFlvPacketCb error status:%d", m_client.status);
        return -1;
    }
    size_t unsend = 0;

    while (m_client.rtmp && (unsend = aio_rtmp_client_get_unsend(m_client.rtmp)) > ZC_RTMP_CLI_UNSENDBUF_BLOCK) {
        system_sleep(100);  // can't send?
        LOG_TRACE("_onFlvPacketCb status:%d, unsend:%u", m_client.status, unsend);
    }

    int ret = 0;
    if (FLV_TYPE_AUDIO == type) {
        ret = aio_rtmp_client_send_audio(m_client.rtmp, data, bytes, timestamp);
    } else if (FLV_TYPE_VIDEO == type) {
#if 0  // ZC_DEBUG
        int keyframe = 1 == (((*(unsigned char *)data) & 0xF0) >> 4);
        if (keyframe)
            LOG_TRACE("type:%02d [A:%d, V:%d, S:%d] key:%d", type, FLV_TYPE_AUDIO, FLV_TYPE_VIDEO, FLV_TYPE_SCRIPT,
                      (type == FLV_TYPE_VIDEO) ? keyframe : 0);
#endif
        ret = aio_rtmp_client_send_video(m_client.rtmp, data, bytes, timestamp);
    } else if (FLV_TYPE_SCRIPT == type) {
        ret = aio_rtmp_client_send_script(m_client.rtmp, data, bytes, timestamp);
    } else {
        ZC_ASSERT(0);
        ret = 0;  // ignore
    }

    return ret;
}

bool CRtmpPushAioCli::_startFlvMuxer() {
    m_flv = new CFlvMuxer();
    if (!m_flv) {
        return false;
    }
    zc_flvmuxer_info_t info = {};
    info.onflvpacketcb = onFlvPacketCb;
    info.Context = this;
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

bool CRtmpPushAioCli::_stopFlvMuxer() {
    if (m_flv) {
        m_flv->Stop();
        m_flv->Destroy();
        delete m_flv;
        m_flv = nullptr;
    }

    return true;
}

int CRtmpPushAioCli::_cliwork() {
    // start
    int ret = 0;
    ret = aio_socket_init(1);
    if (ret < 0) {
        LOG_ERROR("aio_socket_init error", ret);
        return ret;
    }

    m_client.status = 0;
    ret = aio_connect(m_rtmpurl.host, 1935, 3000, aioOnConnect, this);
    if (ret < 0) {
        LOG_ERROR("aio_connect error", ret);
        goto _err;
    }

    while (State() == Running && m_client.status >= 0) {
        ret = aio_socket_process(5000);
        if (ret > 0) {
            aio_timeout_process();
        } else if (ret < 0) {
            break;
        }
    }

    _stopFlvMuxer();

_err:
    // stop
    aio_socket_clean();
    return ret;
}

int CRtmpPushAioCli::process() {
    LOG_WARN("process into");
    while (State() == Running ) {
        if (_cliwork() < 0) {
            break;
        }
        system_sleep(1000);
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
