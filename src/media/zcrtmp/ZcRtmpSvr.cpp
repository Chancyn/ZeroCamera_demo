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

#include "aio-rtmp-server.h"
#include "aio-timeout.h"
#include "aio-worker.h"
#include "flv-demuxer.h"
#include "flv-muxer.h"
#include "flv-proto.h"
#include "flv-writer.h"
#include "sockpair.h"
#include "sockutil.h"
#include "sys/sync.hpp"
#include "sys/system.h"
#include "uri-parse.h"
#include "urlcodec.h"
#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcRtmpSvr.hpp"
#include "ZcType.hpp"
#include "zc_h26x_sps_parse.h"

#include "aio-rtmp-server.h"
#include "aio-timeout.h"
#include "aio-worker.h"
#include "cpm/shared_ptr.h"
#include "flv-demuxer.h"
#include "flv-muxer.h"
#include "flv-proto.h"
#include "flv-writer.h"
#include "sys/sync.hpp"
#include <assert.h>
#include <list>
#include <map>
#include <stdio.h>
#include <string.h>
#include <string>

#define ZC_RTMP_N_AIO_THREAD (4)  // aio thread num
#define ZC_RTMP_CLI_BUF_SIZE (2 * 1024 * 1024)

namespace zc {

CRtmpPlayer::CRtmpPlayer(const aio_rtmp_session_t *rtmp) : m_rtmp(rtmp) {
    m_muxer = flv_muxer_create(&CRtmpPlayer::onHandler, this);
    LOG_TRACE("Constructor %p", this);
}

CRtmpPlayer::~CRtmpPlayer() {
    if (m_muxer)
        flv_muxer_destroy(m_muxer);
    LOG_TRACE("Destructor %p", this);
}

int CRtmpPlayer::onHandler(void *param, int type, const void *data, size_t bytes, uint32_t timestamp) {
    CRtmpPlayer *player = reinterpret_cast<CRtmpPlayer *>(param);
    return player->_onHandler(type, data, bytes, timestamp);
}

int CRtmpPlayer::_onHandler(int type, const void *data, size_t bytes, uint32_t timestamp) {
    struct aio_rtmp_session_t *session = const_cast<struct aio_rtmp_session_t *>(m_rtmp);
    switch (type) {
    case FLV_TYPE_SCRIPT:
        return aio_rtmp_server_send_script(session, data, bytes, timestamp);
    case FLV_TYPE_AUDIO:
        return aio_rtmp_server_send_audio(session, data, bytes, timestamp);
    case FLV_TYPE_VIDEO:
        return aio_rtmp_server_send_video(session, data, bytes, timestamp);
    default:
        assert(0);
        return -1;
    }

    return -1;
}

int CRtmpPlayer::FlvMuxer(int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags) {
    int ret = 0;
    // TODO: push to packet queue
    switch (codec) {
    case FLV_VIDEO_H264:
        ret = flv_muxer_avc(m_muxer, data, bytes, pts, dts);
        break;
    case FLV_VIDEO_H265:
        ret = flv_muxer_hevc(m_muxer, data, bytes, pts, dts);
        break;
    case FLV_AUDIO_AAC:
        ret = flv_muxer_aac(m_muxer, data, bytes, pts, dts);
        break;
    case FLV_AUDIO_MP3:
        ret = flv_muxer_mp3(m_muxer, data, bytes, pts, dts);
        break;
    case FLV_VIDEO_AVCC:
    case FLV_VIDEO_HVCC:
    case FLV_AUDIO_ASC:
        break;  // ignore

    default:
        LOG_DEBUG("error codec:%d", codec);
        ret = 0;
    }

    return ret;
}

int CRtmpPlayer::Pause(int pause, uint32_t ms) {
    // TODO(zhoucc): pause

    return -1;
}

int CRtmpPlayer::Seek(uint32_t ms) {
    // TODO(zhoucc): seek

    return -1;
}

CRtmpSource::CRtmpSource() {
    m_demuxer = flv_demuxer_create(CRtmpSource::onHandler, this);
    m_players.clear();
    LOG_TRACE("Constructor %p", this);
}

CRtmpSource::~CRtmpSource() {
     m_players.clear();
    if (m_demuxer)
        flv_demuxer_destroy(m_demuxer);
    LOG_TRACE("Destructor %p", this);
}

int CRtmpSource::onHandler(void *param, int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts,
                           int flags) {
    CRtmpSource *source = reinterpret_cast<CRtmpSource *>(param);
    return source->_onHandler(codec, data, bytes, pts, dts, flags);
}

int CRtmpSource::_onHandler(int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags) {
    int ret = 0;
    std::lock_guard<std::mutex> locker(m_mutex);
    for (auto it = m_players.begin(); it != m_players.end(); ++it) {
        ret =  it->get()->FlvMuxer(codec, data, bytes, pts, dts, flags);
    }

    // LOG_TRACE("_onHandler(ret:%d, code:%d,bytes: %zu, pts:%u)", ret, codec, bytes, pts);

    return ret;  // ignore error
}

int CRtmpSource::FlvDemuxerInput(int type, const void *data, size_t bytes, uint32_t timestamp) {
    if (!m_demuxer) {
        return -1;
    }
    int ret = flv_demuxer_input(m_demuxer, type, data, bytes, timestamp);
    // LOG_TRACE("FlvDemuxerInput(tret:%d, ype:%d,bytes: %zu, pts:%u)", ret, type, bytes, timestamp);
    return ret;
}

const CRtmpPlayer *CRtmpSource::OpenPlayer(const aio_rtmp_session_t *session) {
    if (m_demuxer) {
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            for (auto it = m_players.begin(); it != m_players.end(); ++it) {
                if (it->get()->GetSession() == session) {
                    LOG_INFO("rtmp session(%p) exist\n", session);
                    return it->get();
                }
            }
        }

        std::shared_ptr<CRtmpPlayer> player(new CRtmpPlayer(session));
        {
            LOG_INFO("player rtmp session(%p) exist\n", session);
            std::lock_guard<std::mutex> locker(m_mutex);
            m_players.push_back(player);
        }
        return player.get();
    }

    return nullptr;
}

bool CRtmpSource::ClosePlayer(const CRtmpPlayer *player) {
    if (m_demuxer) {
        std::lock_guard<std::mutex> locker(m_mutex);
        for (auto it = m_players.begin(); it != m_players.end(); ++it) {
            if (it->get() == player) {
                m_players.erase(it);
                LOG_INFO("ClosePlayer rtmp player(%p)\n", player);
                return true;
            }
        }
    }

    return false;
}

void *CRtmpSvr::onPublish(void *param, aio_rtmp_session_t *session, const char *app, const char *stream,
                          const char *type) {
    CRtmpSvr *svr = reinterpret_cast<CRtmpSvr *>(param);
    return svr->_onPublish(session, app, stream, type);
}

void *CRtmpSvr::_onPublish(aio_rtmp_session_t *session, const char *app, const char *stream, const char *type) {
    LOG_TRACE("_onPublish(%s, %s, %s)", app, stream, type);
    std::string key(app);
    key += "/";
    key += stream;

    std::shared_ptr<CRtmpSource> source(new CRtmpSource());
    std::lock_guard<std::mutex> locker(m_mutex);
    assert(m_livesmap.find(key) == m_livesmap.end());
    m_livesmap[key] = source;
    return source.get();
}

int CRtmpSvr::onScript(void *sourceptr, const void *script, size_t bytes, uint32_t timestamp) {
    ZC_ASSERT(sourceptr);
    CRtmpSource *soruce = reinterpret_cast<CRtmpSource *>(sourceptr);
    return soruce->FlvDemuxerInput(FLV_TYPE_SCRIPT, script, bytes, timestamp);
}

int CRtmpSvr::onVideo(void *sourceptr, const void *data, size_t bytes, uint32_t timestamp) {
    ZC_ASSERT(sourceptr);
    CRtmpSource *soruce = reinterpret_cast<CRtmpSource *>(sourceptr);
    return soruce->FlvDemuxerInput(FLV_TYPE_VIDEO, data, bytes, timestamp);
}

int CRtmpSvr::onAudio(void *sourceptr, const void *data, size_t bytes, uint32_t timestamp) {
    ZC_ASSERT(sourceptr);
    CRtmpSource *soruce = reinterpret_cast<CRtmpSource *>(sourceptr);
    return soruce->FlvDemuxerInput(FLV_TYPE_AUDIO, data, bytes, timestamp);
}

void CRtmpSvr::onSend(void *ptr, size_t bytes) {
    // TODO(zhoucc): todo

    return;
}
void CRtmpSvr::onClose(void *param, void *sourceptr) {
    ZC_ASSERT(sourceptr);
    CRtmpSvr *svr = reinterpret_cast<CRtmpSvr *>(param);
    return svr->_onClose(sourceptr);
}

void CRtmpSvr::_onClose(void *sourceptr) {
    LOG_TRACE("onClose rtmp sourceptr(%p)", sourceptr);
    std::lock_guard<std::mutex> locker(m_mutex);
    for (auto it = m_livesmap.begin(); it != m_livesmap.end(); ++it) {
        std::shared_ptr<CRtmpSource> &source = it->second;
        // try find source
        if (sourceptr == source.get()) {
            m_livesmap.erase(it);
            LOG_TRACE("close source ok");
            return;
        }

        // try find player
        const CRtmpPlayer *payer = reinterpret_cast<const CRtmpPlayer *>(sourceptr);
        if (source.get()->ClosePlayer(payer)) {
            LOG_TRACE("close player ok");
            return;
        }
    }
    return;
}

void *CRtmpSvr::onPlay(void *param, aio_rtmp_session_t *session, const char *app, const char *stream, double start,
                       double duration, uint8_t reset) {
    CRtmpSvr *svr = reinterpret_cast<CRtmpSvr *>(param);
    return svr->_onPlay(session, app, stream, start, duration, reset);
}

void *CRtmpSvr::_onPlay(aio_rtmp_session_t *session, const char *app, const char *stream, double start, double duration,
                        uint8_t reset) {
    LOG_TRACE("_onPlay (%s, %s, %f, %f, %d)", app, stream, start, duration, (int)reset);
    std::string key(app);
    key += "/";
    key += stream;

    std::shared_ptr<CRtmpSource> source;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        auto it = m_livesmap.find(key);
        if (it == m_livesmap.end()) {
            LOG_ERROR("source(%s, %s) not found", app, stream);
            return nullptr;
        }
        source = it->second;
        if (source.get()) {
            return (void *)source.get()->OpenPlayer(session);
        }
    }

    return nullptr;
}

int CRtmpSvr::onPause(void *ptr, int pause, uint32_t ms) {
    CRtmpPlayer *player = reinterpret_cast<CRtmpPlayer *>(ptr);
    return player->Pause(pause, ms);
}

int CRtmpSvr::onSeek(void *ptr, uint32_t ms) {
    CRtmpPlayer *player = reinterpret_cast<CRtmpPlayer *>(ptr);
    return player->Seek(ms);
}

CRtmpSvr::CRtmpSvr()
    : Thread("Rtmpsvr"), m_init(false), m_running(0), m_status(0), m_aiothreadnum(ZC_RTMP_N_AIO_THREAD),
      m_port(ZC_RTMP_PORT), m_phandle(nullptr), m_rtmpsvr(nullptr) {
    strncpy(m_host, "0.0.0.0", sizeof(m_host));
}

CRtmpSvr::~CRtmpSvr() {
    UnInit();
}

bool CRtmpSvr::Init(ZC_U16 port) {
    if (m_init) {
        return false;
    }

    m_port = port;
    m_init = true;
    return true;
}

bool CRtmpSvr::UnInit() {
    Stop();
    m_init = false;
    return true;
}
bool CRtmpSvr::Start() {
    if (m_running) {
        LOG_ERROR("already Start");
        return false;
    }

    Thread::Start();
    m_running = true;
    LOG_TRACE("Start ok");
    return true;
}

bool CRtmpSvr::Stop() {
    LOG_TRACE("Stop into");
    if (m_running) {
        Thread::Stop();
        m_running = false;
    }
    LOG_TRACE("Stop ok");
    return true;
}

bool CRtmpSvr::_startsvr() {
    int ret = 0;
    struct aio_rtmp_server_handler_t *phandle = nullptr;
    struct aio_rtmp_server_t *rtmp = nullptr;

    LOG_TRACE("rtmpsvr, host:%s, port:%hu", m_host, m_port);
    phandle = (struct aio_rtmp_server_handler_t *)malloc(sizeof(struct aio_rtmp_server_handler_t));
    ZC_ASSERT(phandle);
    if (!phandle) {
        LOG_ERROR("rtmpsvr malloc error m_port[%s] this[%p]", m_port, this);
        goto _err;
    }
    m_phandle = phandle;
    memset(phandle, 0, sizeof(*phandle));
    phandle->onsend = onSend;
    phandle->onplay = onPlay;
    phandle->onpause = onPause;
    phandle->onseek = onSeek;
    phandle->onpublish = onPublish;
    phandle->onscript = onScript;
    phandle->onaudio = onAudio;
    phandle->onvideo = onVideo;
    phandle->onclose = onClose;

    aio_worker_init(8);
    rtmp = aio_rtmp_server_create(m_host, m_port, phandle, this);
    if (!rtmp) {
        LOG_ERROR("rtmp rtmp_client_create error");
        ZC_ASSERT(0);
        goto _err;
    }

    m_rtmpsvr = rtmp;

    m_status = 1;
    LOG_TRACE("rtmpsvr starcomm ok");
    return true;
_err:
    _stopsvr();
    m_status = 0;
    LOG_ERROR("rtmpsvr _startconn error");
    return false;
}

bool CRtmpSvr::_stopsvr() {
    LOG_TRACE("_stopsvr into");
    if (m_rtmpsvr) {
        aio_rtmp_server_destroy(m_rtmpsvr);
        m_rtmpsvr = nullptr;
        ZC_SAFE_FREE(m_phandle);
        aio_worker_clean(m_aiothreadnum);
    }

    LOG_TRACE("_stopsvr ok");
    return true;
}

int CRtmpSvr::_svrwork() {
    // start
    int ret = 0;
    if (!_startsvr()) {
        LOG_ERROR("_startconn error");
        return -1;
    }

    while (State() == Running && m_status == 1) {
        system_sleep(5);
    }
    LOG_TRACE("_svrwork loop end");
_err:
    // stop
    _stopsvr();
    LOG_ERROR("_svrwork exit");
    return ret;
}

int CRtmpSvr::process() {
    LOG_WARN("process into");
    while (State() == Running /*&&  i < loopcnt*/) {
        if (_svrwork() < 0) {
            break;
        }
        system_sleep(1000);
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
