// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "aio-rtmp-server.h"
#include "sockutil.h"
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcFlvMuxer.hpp"

#define ZC_RTMP_PORT (1935)       // 1935

namespace zc {
class CRtmpPlayer {
 public:
    explicit CRtmpPlayer(const aio_rtmp_session_t *rtmp);
    virtual ~CRtmpPlayer();

    const aio_rtmp_session_t *GetSession() { return m_rtmp; }
    int Pause(int pause, uint32_t ms);
    int Seek(uint32_t ms);
    int FlvMuxer(int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags);

 private:
    static int onHandler(void *param, int type, const void *data, size_t bytes, uint32_t timestamp);
    int _onHandler(int type, const void *data, size_t bytes, uint32_t timestamp);

 private:
    // TODO(zhoucc): add packet queue
    const aio_rtmp_session_t *m_rtmp;
    struct flv_muxer_t *m_muxer;
};

class CRtmpSvr;
class CRtmpSource {
 public:
    CRtmpSource();
    virtual ~CRtmpSource();

 public:
    int FlvDemuxerInput(int type, const void *data, size_t bytes, uint32_t timestamp);
    const CRtmpPlayer *OpenPlayer(const aio_rtmp_session_t *session);
    bool ClosePlayer(const CRtmpPlayer *player);

 private:
    static int onHandler(void *param, int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags);
    int _onHandler(int codec, const void *data, size_t bytes, uint32_t pts, uint32_t dts, int flags);

 private:
    std::mutex m_mutex;
    struct flv_demuxer_t *m_demuxer;
    std::list<std::shared_ptr<CRtmpPlayer>> m_players;
};

class CRtmpSvr : protected Thread {
 public:
    CRtmpSvr();
    virtual ~CRtmpSvr();

 public:
    bool Init(ZC_U16 port = ZC_RTMP_PORT);
    bool UnInit();
    bool Start();
    bool Stop();
 private:
    bool _startsvr();
    bool _stopsvr();
    int _svrwork();
    virtual int process();

    static void *onPublish(void *param, aio_rtmp_session_t *session, const char *app, const char *stream,
                           const char *type);
    void *_onPublish(aio_rtmp_session_t *session, const char *app, const char *stream, const char *type);
    static int onScript(void *sourceptr, const void *script, size_t bytes, uint32_t timestamp);
    static int onVideo(void *sourceptr, const void *data, size_t bytes, uint32_t timestamp);
    static int onAudio(void *sourceptr, const void *data, size_t bytes, uint32_t timestamp);
    static void onSend(void *ptr, size_t bytes);
    static void onClose(void *param, void *sourceptr);
    void _onClose(void *sourceptr);
    static void *onPlay(void *param, aio_rtmp_session_t *session, const char *app, const char *stream, double start,
                        double duration, uint8_t reset);
    void *_onPlay(aio_rtmp_session_t *session, const char *app, const char *stream, double start, double duration,
                  uint8_t reset);

    static int onPause(void *ptr, int pause, uint32_t ms);
    static int onSeek(void *ptr, uint32_t ms);

 private:
    bool m_init;
    int m_running;
    int m_status;
    ZC_U8 m_aiothreadnum;
    ZC_U16 m_port;
    char m_host[ZC_MAX_PATH];
    void *m_phandle;
    aio_rtmp_server_t *m_rtmpsvr;
    std::mutex m_mutex;
    std::map<std::string, std::shared_ptr<CRtmpSource>> m_livesmap;
};

}  // namespace zc
