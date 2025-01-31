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

#include "ZcType.hpp"
#include "cpm/unuse.h"
#include "cstringext.h"
#include "aio-connect.h"
#include "aio-rtmp-client.h"
#include "aio-timeout.h"
#include "flv-proto.h"
#include "flv-reader.h"
#include "sys/system.h"
#include "uri-parse.h"
#include "urlcodec.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcRtmpPullCli.hpp"
#include "zc_h26x_sps_parse.h"

#define ZC_RTSP_CLI_BUF_SIZE (2 * 1024 * 1024)

namespace zc {
CRtmpPullCli::CRtmpPullCli() : Thread("RtmpPull"), m_init(false), m_running(0), m_phandle(nullptr),m_flvmuxer(nullptr), m_streamsput(nullptr)  {
    memset(&m_client, 0, sizeof(m_client));
}

CRtmpPullCli::~CRtmpPullCli() {
    UnInit();
}
#if 0
int CRtmpPullCli::_onsetup(int timeout, int64_t duration) {
    uint64_t npt = 0;
    char ip[65];
    u_short rtspport;
    int ret = 0;

    zc_stream_info_t stinfo = {0};
    // get streaminfo
    if (!m_cbinfo.GetInfoCb || m_cbinfo.GetInfoCb(m_cbinfo.MgrContext, m_chn, &stinfo) < 0) {
        LOG_ERROR("rtsp_client_play m_cbinfo.GetInfoCb error");
        return -1;
    }

    // update streaminfo
    if (!m_cbinfo.SetInfoCb || m_cbinfo.SetInfoCb(m_cbinfo.MgrContext, m_chn, &stinfo) < 0) {
        LOG_ERROR("rtsp_client_play m_cbinfo.GetInfoCb  ret[%d]", ret);
        return 0;
    }

    return 0;
}
#endif

int CRtmpPullCli::rtmpClientPullOnVideo(void *ptr, const void *data, size_t bytes, uint32_t timestamp) {
    CRtmpPullCli *pcli = reinterpret_cast<CRtmpPullCli *>(ptr);
    return pcli->_rtmpClientPullOnVideo(data, bytes, timestamp);
}

int CRtmpPullCli::_rtmpClientPullOnVideo(const void *data, size_t bytes, uint32_t timestamp) {
    // LOG_TRACE("rtmpull onvideo bytes:%u, timestamp:%u", bytes, timestamp);

    return m_flvmuxer->Input(FLV_TYPE_VIDEO, (const uint8_t *)data, bytes, timestamp);
}

int CRtmpPullCli::rtmpClientPullOnAudio(void *ptr, const void *data, size_t bytes, uint32_t timestamp) {
    CRtmpPullCli *pcli = reinterpret_cast<CRtmpPullCli *>(ptr);
    return pcli->_rtmpClientPullOnAudio(data, bytes, timestamp);
}

int CRtmpPullCli::_rtmpClientPullOnAudio(const void *data, size_t bytes, uint32_t timestamp) {
    // LOG_TRACE("rtmpull onaudio bytes:%u, timestamp:%u", bytes, timestamp);
    return m_flvmuxer->Input(FLV_TYPE_AUDIO, (const uint8_t *)data, bytes, timestamp);
}

int CRtmpPullCli::rtmpClientPullOnScript(void *ptr, const void *data, size_t bytes, uint32_t timestamp) {
    CRtmpPullCli *pcli = reinterpret_cast<CRtmpPullCli *>(ptr);
    return pcli->_rtmpClientPullOnScript(data, bytes, timestamp);
}

int CRtmpPullCli::_rtmpClientPullOnScript(const void *data, size_t bytes, uint32_t timestamp) {
    // LOG_TRACE("rtmpull onscript bytes:%u, timestamp:%u", bytes, timestamp);

    return m_flvmuxer->Input(FLV_TYPE_SCRIPT, (const uint8_t *)data, bytes, timestamp);
}

void CRtmpPullCli::rtmpClientPullOnError(void *ptr, int code) {
    return reinterpret_cast<CRtmpPullCli *>(ptr)->_rtmpClientPullOnError(code);
}

void CRtmpPullCli::_rtmpClientPullOnError(int code) {
    LOG_TRACE("rtmppull onerror, code:%d", code);

    _stopFlvDemuxer();
    m_client.status = -1;
    LOG_ERROR("PullOnError error, _stopFlvMuxer set status %d", m_client.status);

    return;
}

bool CRtmpPullCli::Init(rtmppull_callback_info_t *cbinfo, int chn, const char *url) {
    if (m_init || !url || url[0] == '\0') {
        return false;
    }

    m_chn = chn;
    memcpy(&m_cbinfo, cbinfo, sizeof(rtmppull_callback_info_t));
    strncpy(m_url, url, sizeof(m_url));
    // memcpy(&m_info, &info, sizeof(zc_stream_info_t));
    m_streamsput = new CStreamsPut();
    if (!m_streamsput) {
        LOG_TRACE("new CStreamsPut");
        return false;
    }

    if (!m_streamsput->Init(chn)) {
        LOG_ERROR("CStreamsPut Init error");
        goto _err;
    }

    m_init = true;
    return true;
_err:
    LOG_ERROR("error");
    ZC_SAFE_DELETE(m_streamsput);
    return false;
}

bool CRtmpPullCli::UnInit() {
    if (m_init) {
        StopCli();
        ZC_SAFE_DELETE(m_streamsput);
        m_init = false;
    }

    return true;
}

void CRtmpPullCli::aioOnConnect(void *ptr, int code, socket_t tcp, aio_socket_t aio) {
    CRtmpPullCli *pcli = reinterpret_cast<CRtmpPullCli *>(ptr);
    return pcli->_aioOnConnect(code, tcp, aio);
}

// rtmp://video-center.alivecdn.com/live/hello?vhost=your.domain
// rtmp_pull_test("video-center.alivecdn.com", "live", "hello?vhost=your.domain", local-flv-file-name)
void CRtmpPullCli::_aioOnConnect(int code, socket_t tcp, aio_socket_t aio) {
    if (code != 0) {
        m_client.status = -1;
        LOG_TRACE("aioOnConnect error, code:%d", code);
        return;
    }

    if (!_startFlvDemuxer()) {
        m_client.status = -1;
        LOG_TRACE("aioOnConnect _startFlvDemuxer");
        return;
    }

    struct aio_rtmp_client_handler_t *phandle = nullptr;
    struct aio_rtmp_client_t *rtmp = nullptr;
    LOG_TRACE("rtmppull url:%s, host:%s, port:%hu, app:%s, stream:%s", m_rtmpurl.rurl, m_rtmpurl.host, m_rtmpurl.port,
              m_rtmpurl.app, m_rtmpurl.stream);
    phandle = (struct aio_rtmp_client_handler_t *)malloc(sizeof(struct aio_rtmp_client_handler_t));
    ZC_ASSERT(phandle);
    if (!phandle) {
        LOG_ERROR("rtmppull malloc error url[%s] this[%p]", m_url, this);
        goto _err;
    }
    memset(phandle, 0, sizeof(aio_rtmp_client_handler_t));
    m_phandle = phandle;
    phandle->onerror = rtmpClientPullOnError;
    phandle->onaudio = rtmpClientPullOnAudio;
    phandle->onvideo = rtmpClientPullOnVideo;
    phandle->onscript = rtmpClientPullOnScript;

    rtmp = aio_rtmp_client_create(aio, m_rtmpurl.app, m_rtmpurl.stream, m_rtmpurl.rurl, phandle, this);
    if (!rtmp) {
        LOG_ERROR("rtmp aio_rtmp_client_create error");
        ZC_ASSERT(0);
        goto _err;
    }

    m_client.rtmp = rtmp;
    if (aio_rtmp_client_start(rtmp, 1) < 0) {
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

bool CRtmpPullCli::StartCli() {
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

bool CRtmpPullCli::StopCli() {
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

bool CRtmpPullCli::_stopconn() {
    if (m_client.rtmp) {
        aio_rtmp_client_destroy(reinterpret_cast<aio_rtmp_client_t *>(m_client.rtmp));
        m_client.rtmp = nullptr;
    }

    ZC_SAFE_FREE(m_phandle);
    return true;
}

int CRtmpPullCli::onFrameCb(void *ptr, zc_frame_t *framehdr, const uint8_t *data) {
    return reinterpret_cast<CRtmpPullCli *>(ptr)->_onFrameCb(framehdr, data);
}

int CRtmpPullCli::_onFrameCb(zc_frame_t *framehdr, const uint8_t *data) {
    // LOG_TRACE("_onFrameCb type:%u, code:%u, seqno:%u, size:%u, pts:%u, keyflag:%d", framehdr->type, framehdr->video.encode,
    //           framehdr->seq, framehdr->size, framehdr->pts, framehdr->keyflag);
    return m_streamsput->PutFrame(framehdr, data);
}

bool CRtmpPullCli::_startFlvDemuxer() {
    zc_flvdemuxer_info_t info = {};
    info.onframe = onFrameCb;
    info.ctx = this;
    if (m_flvmuxer) {
        delete m_flvmuxer;
        m_flvmuxer = nullptr;
    }

    m_flvmuxer = new CFlvDemuxer(info);
    if (!m_flvmuxer) {
        LOG_ERROR("new CFlvDemuxer error");
        return false;
    }

    LOG_TRACE("startFlvMuxer OK");
    return true;
}

bool CRtmpPullCli::_stopFlvDemuxer() {
    if (m_flvmuxer) {
        delete m_flvmuxer;
        m_flvmuxer = nullptr;
        LOG_ERROR("_stopFlvDemuxer ok");
    }

    return true;
}

int CRtmpPullCli::_cliwork() {
    // start
    int ret = 0;
    ret = aio_socket_init(1);
    if (ret < 0) {
        LOG_ERROR("aio_socket_init error", ret);
        return ret;
    }
    m_client.status = 0;
    LOG_TRACE("rtmppull connect url:%s, host:%s, port:%hu, app:%s, stream:%s", m_rtmpurl.rurl, m_rtmpurl.host, m_rtmpurl.port,
              m_rtmpurl.app, m_rtmpurl.stream);
    ret = aio_connect(m_rtmpurl.host, m_rtmpurl.port, 3000, aioOnConnect, this);
    if (ret < 0) {
        LOG_ERROR("aio_connect error", ret);
        goto _err;
    }

    while (State() == Running && m_client.status >= 0) {
        ret = aio_socket_process(5000);
        if (ret > 0) {
            aio_timeout_process();
        } else if (ret < 0) {
            LOG_ERROR("aio_socket_process error", ret);
            break;
        }
    }

    _stopFlvDemuxer();

_err:
    // stop
    aio_socket_clean();
    return ret;
}

int CRtmpPullCli::process() {
    LOG_WARN("process into");
    while (State() == Running) {
        if (_cliwork() < 0) {
            break;
        }
        system_sleep(1000);
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
