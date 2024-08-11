// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <list>
#include <mutex>

#include "zc_type.h"

#include "NonCopyable.hpp"
#include "Thread.hpp"
#include "ZcWebMSess.hpp"
#include "ZcFlvSess.hpp"
#include "ZcFmp4Sess.hpp"
#include "ZcTsSess.hpp"


#define ZC_SUPPORT_SSL 1

#define ZC_WEBS_WORKPATH_DEF "./"    // work path
#define ZC_WEBS_HTTP_PORT_DEF 8000   // 80
#define ZC_WEBS_HTTPS_PORT_DEF 8443  // http
#define ZC_WEBS_WS_PORT_DEF 8088     // ws
#define ZC_WEBS_WSS_PORT_DEF 8444    // wss

// type
typedef enum {
    zc_webs_type_http = 0,  // bit0 http
    zc_webs_type_ws,        // bit2 ws websocket
    zc_webs_type_https,     // bit1 https
    zc_webs_type_wss,       // bit3 wss websocket ssl

    zc_webs_type_butt,  // max
} zc_webs_type_e;

namespace zc {
typedef struct _zc_webs_info {
    ZC_U16 local;  // 0:server (0.0.0.0); 1:local server(127.0.0.1)
    ZC_U16 port;
    ZC_CHAR workpath[128];  // path
} zc_webs_info_t;

typedef int (*WebSvrGetStreamInfoCb)(void *ptr, unsigned int type, unsigned int chn, zc_stream_info_t *info);
// Cstruct web callback
typedef struct {
    WebSvrGetStreamInfoCb getStreamInfoCb;
    void *MgrContext;
} websvr_cb_info_t;

class CWebServer : protected Thread, public NonCopyable {
 public:
    explicit CWebServer(zc_webs_type_e type, const websvr_cb_info_t &cbinfo);
    virtual ~CWebServer();

 public:
    bool Init(zc_webs_info_t *info = nullptr);
    bool UnInit();
    bool Start();
    bool Stop();

 protected:
    virtual void EventHandler(struct mg_connection *c, int ev, void *ev_data);

 private:
    int unInitFlvSess();
    static void InitMgLog();
    virtual int process();
    static void EventHandlerCb(struct mg_connection *c, int ev, void *ev_data);
    static int sendFlvDataCb(void *ptr, void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);
    int _sendFlvDataCb(void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);
    int handleOpenHttpFlvSession(struct mg_connection *c, void *ev_data, zc_shmstream_e type, int chn);
    int _sendFlvHdr(void *sess, bool hasvideo, bool hasaudio);
    int handleCloseHttpFlvSession(struct mg_connection *c, void *ev_data);
    static int sendWsFlvDataCb(void *ptr, void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);
    int _sendWsFlvDataCb(void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);
    int _sendWsFlvHdr(void *sess, bool hasvideo, bool hasaudio);
    int handleOpenWsFlvSession(struct mg_connection *c, void *ev_data, zc_shmstream_e type, int chn);

    // fmp4
    static int sendFmp4DataCb(void *ptr, void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);
    int _sendFmp4DataCb(void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);
    int handleOpenHttpFmp4Session(struct mg_connection *c, void *ev_data, zc_shmstream_e type, int chn);
    int handleOpenWsFmp4Session(struct mg_connection *c, void *ev_data, zc_shmstream_e type, int chn);

    // ts
    static int sendTsDataCb(void *ptr, void *sess, const void *data, size_t bytes);
    int _sendTsDataCb(void *sess, const void *data, size_t bytes);
    int handleOpenHttpTsSession(struct mg_connection *c, void *ev_data, zc_shmstream_e type, int chn);
    int handleOpenWsTsSession(struct mg_connection *c, void *ev_data, zc_shmstream_e type, int chn);

    int handleOpenHttpMediaSession(struct mg_connection *c, void *ev_data, zc_web_msess_type_e mtype,
                                   zc_shmstream_e type, int chn);
    int handleOpenWsMediaSession(struct mg_connection *c, void *ev_data, zc_web_msess_type_e mtype, zc_shmstream_e type,
                                 int chn);

 private:
    static bool m_mgloginit;
    bool m_init;
    bool m_running;
    bool m_ssl;
    zc_webs_type_e m_type;
    zc_webs_info_t m_info;
    websvr_cb_info_t m_cbinfo;
    char m_addrs[32];   // https server addr
    void *m_mgrhandle;  // mongoose mgr
    std::mutex m_flvsessmutex;
    std::list<CFlvSess *> m_flvsesslist;
    std::list<CFmp4Sess *> m_fmp4sesslist;
    std::list<CTsSess *> m_tssesslist;
};

class CWebServerHttp : public CWebServer {
 public:
    explicit CWebServerHttp(const websvr_cb_info_t &cbinfo) : CWebServer(zc_webs_type_http, cbinfo) {}
    virtual ~CWebServerHttp() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerHttps : public CWebServer {
 public:
    explicit CWebServerHttps(const websvr_cb_info_t &cbinfo) : CWebServer(zc_webs_type_https, cbinfo) {}
    virtual ~CWebServerHttps() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerWS : public CWebServer {
 public:
    explicit CWebServerWS(const websvr_cb_info_t &cbinfo) : CWebServer(zc_webs_type_ws, cbinfo) {}
    virtual ~CWebServerWS() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerWSS : public CWebServer {
 public:
    explicit CWebServerWSS(const websvr_cb_info_t &cbinfo) : CWebServer(zc_webs_type_wss, cbinfo) {}
    virtual ~CWebServerWSS() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerFac {
 public:
    CWebServerFac() {}
    ~CWebServerFac() {}
    static CWebServer *CreateWebServer(zc_webs_type_e type, const websvr_cb_info_t &cbinfo) {
        CWebServer *webs = nullptr;
        printf("CreateWebServer into, type:%d", type);
        switch (type) {
        case zc_webs_type_http:
            webs = new CWebServerHttp(cbinfo);
            break;
        case zc_webs_type_ws:
            webs = new CWebServerWS(cbinfo);
            break;
#if ZC_SUPPORT_SSL
        case zc_webs_type_https:
            webs = new CWebServerHttps(cbinfo);
            break;
        case zc_webs_type_wss:
            webs = new CWebServerWSS(cbinfo);
            break;
#endif
        default:
            break;
        }
        return webs;
    }
};

}  // namespace zc
