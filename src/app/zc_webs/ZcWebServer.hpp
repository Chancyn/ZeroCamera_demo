// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "NonCopyable.hpp"
#include "zc_type.h"

#include "NonCopyable.hpp"
#include "Thread.hpp"

#define ZC_SUPPORT_SSL 1

#define ZC_WEBS_WORKPATH_DEF "./"    // work path
#define ZC_WEBS_HTTP_PORT_DEF 8000   // 80
#define ZC_WEBS_HTTPS_PORT_DEF 8443  // http
#define ZC_WEBS_WS_PORT_DEF 8080     // ws
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

class CWebServer : protected Thread, public NonCopyable {
 public:
    explicit CWebServer(zc_webs_type_e type);
    virtual ~CWebServer();

 public:
    bool Init(zc_webs_info_t *info = nullptr);
    bool UnInit();
    bool Start();
    bool Stop();

 protected:
    virtual void EventHandler(struct mg_connection *c, int ev, void *ev_data);

 private:
    static void InitMgLog();
    virtual int process();
    static void EventHandlerCb(struct mg_connection *c, int ev, void *ev_data);

 private:
    static bool m_mgloginit;
    bool m_init;
    bool m_running;
    bool m_ssl;
    zc_webs_type_e m_type;
    zc_webs_info_t m_info;
    char m_addrs[32];   // https server addr
    void *m_mgrhandle;  // mongoose mgr
};

class CWebServerHttp : public CWebServer {
 public:
    CWebServerHttp() : CWebServer(zc_webs_type_http) {}
    virtual ~CWebServerHttp() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerHttps : public CWebServer {
 public:
    CWebServerHttps() : CWebServer(zc_webs_type_https) {}
    virtual ~CWebServerHttps() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerWS : public CWebServer {
 public:
    CWebServerWS() : CWebServer(zc_webs_type_ws) {}
    virtual ~CWebServerWS() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerWSS : public CWebServer {
 public:
    CWebServerWSS() : CWebServer(zc_webs_type_wss) {}
    virtual ~CWebServerWSS() {}
    void EventHandler(struct mg_connection *c, int ev, void *ev_data);
};

class CWebServerFac {
 public:
    CWebServerFac() {}
    ~CWebServerFac() {}
    static CWebServer *CreateWebServer(zc_webs_type_e type) {
        CWebServer *webs = nullptr;
        printf("CreateWebServer into, type:%d", type);
        switch (type) {
        case zc_webs_type_http:
            webs = new CWebServerHttp();
            break;
        case zc_webs_type_ws:
            webs = new CWebServerWS();
            break;
#if ZC_SUPPORT_SSL
        case zc_webs_type_https:
            webs = new CWebServerHttps();
            break;
        case zc_webs_type_wss:
            webs = new CWebServerWSS();
            break;
#endif
        default:
            break;
        }
        return webs;
    }
};

}  // namespace zc
