// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "media/ZcMediaReceiver.hpp"
#include "zc_frame.h"
#include "zc_type.h"

#include "rtsp-server-internal.h"
#include "rtsp-server.h"

#include "rtsp-header-transport.h"
#include "rtsp-media.h"

#include "Thread.hpp"
#include "ZcRtpReceiver.hpp"
#include "rtsp/ZcModRtsp.hpp"

#define ZC_RTSP_MAX_CHN ZC_STREAM_VIDEO_MAX_CHN

namespace zc {
typedef int (*RtspPushSGetInfoCb)(void *ptr, unsigned int chn, zc_media_info_t *data);
typedef int (*RtspPushSSetInfoCb)(void *ptr, unsigned int chn, zc_media_info_t *data);

typedef struct {
    RtspPushSGetInfoCb GetInfoCb;
    RtspPushSSetInfoCb SetInfoCb;
    void *MgrContext;
} rtsppushs_callback_info_t;

// TODO(zhoucc) : optimization to cstruct style
struct pushrtsp_stream_t {
    std::shared_ptr<CMediaReceiver> mediarecv;  // media frame receiver
    std::shared_ptr<rtsp_media_t> media;
    std::shared_ptr<CRtpReceiver> rtpreceiver;
    struct rtsp_header_transport_t transport;
    socket_t socket[2];       // rtp/rtcp socket
    unsigned int tack;        // media trackid
    int trackcode;            // media track code
    unsigned int encode;  // media  encode
    unsigned int tracktype;   // media tracktype
};

struct pushrtsp_source_t {
    int count;
    struct rtsp_media_t media[8];
};

struct pushrtsp_session_t {
    // TODO(zhoucc) : optimization to cstruct style, array, to find streams/rtpreceiver quick
    std::list<std::shared_ptr<pushrtsp_stream_t>> streams;
    int status;  // setup-init, 1-play, 2-pause
};

typedef std::map<std::string, std::shared_ptr<pushrtsp_session_t>> TPushSessions;

// static std::map<std::string, std::shared_ptr<pushrtsp_source_t> > s_pushsources;
// static TPushSessions s_sessions;
// static ThreadLocker s_locker;

class CRtspPushServer : public Thread {
 public:
    CRtspPushServer();
    virtual ~CRtspPushServer();

 public:
    bool Init(rtsppushs_callback_info_t *cbinfo);
    bool UnInit();

 private:
    bool _init();
    bool _unInit();
    virtual int process();
    bool _aio_work();
    static int onframe(void *ptr1, void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags);
    int _onframe(void *ptr2, int encode, const void *packet, int bytes, uint32_t time, int flags);
    int rtsp_uri_parse(const char *uri, std::string &path);

    int _pushrtsp_find_media(const char *uri, std::shared_ptr<pushrtsp_source_t> &source);
    static int rtsp_onsetup(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                            const struct rtsp_header_transport_t transports[], size_t num);
    int _onsetup(rtsp_server_t *rtsp, const char *uri, const char *session,
                 const struct rtsp_header_transport_t transports[], size_t num);
    //  static int rtsp_onplay(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
    //                         const double *scale);
    //  int _onplay(rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
    //              const double *scale);
    //  static int rtsp_onpause(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t
    //  *npt); int _onpause(rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt);
    static int rtsp_onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session);
    int _onteardown(rtsp_server_t *rtsp, const char *uri, const char *session);
    static int rtsp_onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp, int len);
    int _onannounce(rtsp_server_t *rtsp, const char *uri, const char *sdp, int len);
    static int rtsp_onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                             const double *scale);
    int _onrecord(rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt, const double *scale);
    static int rtsp_onoptions(void *ptr, rtsp_server_t *rtsp, const char *uri);
    int _onoptions(rtsp_server_t *rtsp, const char *uri);
    static int rtsp_ongetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                   const void *content, int bytes);
    int _ongetparameter(rtsp_server_t *rtsp, const char *uri, const char *session, const void *content, int bytes);
    static int rtsp_onsetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                   const void *content, int bytes);
    int _onsetparameter(rtsp_server_t *rtsp, const char *uri, const char *session, const void *content, int bytes);
    static int rtsp_onclose(void *ptr2);
    int _onclose(void *ptr2);
    static void rtsp_onerror(void *ptr, rtsp_server_t *rtsp, int code);
    void _onerror(rtsp_server_t *rtsp, int code);

    static void rtsp_onerror2(void *ptr, rtsp_server_t *rtsp, int code, void *ptr2);
    void _onerror2(rtsp_server_t *rtsp, int code, void *ptr2);

    static int rtsp_send(void *ptr, const void *data, size_t bytes);
    int _send(const void *data, size_t bytes);

    static void onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes);
    void _onrtp(uint8_t channel, const void *data, uint16_t bytes);
    static void onrtp2(void *param, uint8_t channel, const void *data, uint16_t bytes, void *ptr2);
    void _onrtp2(uint8_t channel, const void *data, uint16_t bytes, void *ptr2);

 private:
    bool m_init;
    int m_running;
    rtsppushs_callback_info_t m_cbinfo;
    void *m_phandle;  // handle
    void *m_psvr;     // server
    std::mutex m_pushmutex;
    std::map<std::string, std::shared_ptr<pushrtsp_source_t>> m_pushsources;
    TPushSessions m_pushsessions;
};
}  // namespace zc
