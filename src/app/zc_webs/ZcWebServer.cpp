// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "Thread.hpp"
#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg_sys.h"

#include "mongoose.h"

#include "ZcType.hpp"
#include "ZcWebServer.hpp"

namespace zc {
const char *g_addrprefix[zc_webs_type_butt] = {
    "http://",
    "ws://",
    "https://",
    "wss://",
};

// Self signed certificates
// https://mongoose.ws/documentation/tutorials/tls/#self-signed-certificates
#ifdef TLS_TWOWAY
static const char *s_tls_ca = "-----BEGIN CERTIFICATE-----\n"
                              "MIIBqjCCAU+gAwIBAgIUESoOPGqMhf9uarzblVFwzrQweMcwCgYIKoZIzj0EAwIw\n"
                              "RDELMAkGA1UEBhMCSUUxDzANBgNVBAcMBkR1YmxpbjEQMA4GA1UECgwHQ2VzYW50\n"
                              "YTESMBAGA1UEAwwJVGVzdCBSb290MCAXDTIwMDUwOTIxNTE0NFoYDzIwNTAwNTA5\n"
                              "MjE1MTQ0WjBEMQswCQYDVQQGEwJJRTEPMA0GA1UEBwwGRHVibGluMRAwDgYDVQQK\n"
                              "DAdDZXNhbnRhMRIwEAYDVQQDDAlUZXN0IFJvb3QwWTATBgcqhkjOPQIBBggqhkjO\n"
                              "PQMBBwNCAAQsq9ECZiSW1xI+CVBP8VDuUehVA166sR2YsnJ5J6gbMQ1dUCH/QvLa\n"
                              "dBdeU7JlQcH8hN5KEbmM9BnZxMor6ussox0wGzAMBgNVHRMEBTADAQH/MAsGA1Ud\n"
                              "DwQEAwIBrjAKBggqhkjOPQQDAgNJADBGAiEAnHFsAIwGQQyRL81B04dH6d86Iq0l\n"
                              "fL8OKzndegxOaB0CIQCPwSIwEGFdURDqCC0CY2dnMrUGY5ZXu3hHCojZGS7zvg==\n"
                              "-----END CERTIFICATE-----\n";
#endif
static const char *s_tls_cert = "-----BEGIN CERTIFICATE-----\n"
                                "MIIBhzCCASygAwIBAgIUbnMoVd8TtWH1T09dANkK2LU6IUswCgYIKoZIzj0EAwIw\n"
                                "RDELMAkGA1UEBhMCSUUxDzANBgNVBAcMBkR1YmxpbjEQMA4GA1UECgwHQ2VzYW50\n"
                                "YTESMBAGA1UEAwwJVGVzdCBSb290MB4XDTIwMDUwOTIxNTE0OVoXDTMwMDUwOTIx\n"
                                "NTE0OVowETEPMA0GA1UEAwwGc2VydmVyMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcD\n"
                                "QgAEkuBGnInDN6l06zVVQ1VcrOvH5FDu9MC6FwJc2e201P8hEpq0Q/SJS2nkbSuW\n"
                                "H/wBTTBaeXN2uhlBzMUWK790KKMvMC0wCQYDVR0TBAIwADALBgNVHQ8EBAMCA6gw\n"
                                "EwYDVR0lBAwwCgYIKwYBBQUHAwEwCgYIKoZIzj0EAwIDSQAwRgIhAPo6xx7LjCdZ\n"
                                "QY133XvLjAgVFrlucOZHONFVQuDXZsjwAiEAzHBNligA08c5U3SySYcnkhurGg50\n"
                                "BllCI0eYQ9ggp/o=\n"
                                "-----END CERTIFICATE-----\n";

static const char *s_tls_key = "-----BEGIN PRIVATE KEY-----\n"
                               "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQglNni0t9Dg9icgG8w\n"
                               "kbfxWSS+TuNgbtNybIQXcm3NHpmhRANCAASS4EacicM3qXTrNVVDVVys68fkUO70\n"
                               "wLoXAlzZ7bTU/yESmrRD9IlLaeRtK5Yf/AFNMFp5c3a6GUHMxRYrv3Qo\n"
                               "-----END PRIVATE KEY-----\n";

// def tab
static zc_webs_info_t g_websinfodeftab[zc_webs_type_butt] = {
    // local, port, workpath
    {0, ZC_WEBS_HTTP_PORT_DEF, ZC_WEBS_WORKPATH_DEF},
    {0, ZC_WEBS_WS_PORT_DEF, ZC_WEBS_WORKPATH_DEF},
    {0, ZC_WEBS_HTTPS_PORT_DEF, ZC_WEBS_WORKPATH_DEF},
    {0, ZC_WEBS_WSS_PORT_DEF, ZC_WEBS_WORKPATH_DEF},
};

bool CWebServer::m_mgloginit = false;

// modsyscli
CWebServer::CWebServer(zc_webs_type_e type)
    : Thread("Webs"), m_init(false), m_running(0), m_type(type), m_mgrhandle(nullptr) {
    memset(&m_info, 0, sizeof(m_info));
    memset(m_addrs, 0, sizeof(m_addrs));
    if (m_type == zc_webs_type_https || m_type == zc_webs_type_wss) {
        m_ssl = true;
    } else {
        m_ssl = false;
    }
}

CWebServer::~CWebServer() {
    UnInit();
}

void CWebServer::EventHandlerCb(struct mg_connection *c, int ev, void *ev_data) {
    CWebServer *webs = reinterpret_cast<CWebServer *>(c->fn_data);
    return webs->EventHandler(c, ev, ev_data);
}

void CWebServer::EventHandler(struct mg_connection *c, int ev, void *ev_data) {
#if ZC_SUPPORT_SSL
    if (ev == MG_EV_ACCEPT && m_ssl) {
        LOG_ERROR("accept ssl");
        struct mg_tls_opts opts = {
#ifdef TLS_TWOWAY
            .ca = mg_str(s_tls_ca),
#endif
            .cert = mg_str(s_tls_cert),
            .key = mg_str(s_tls_key)};
        mg_tls_init(c, &opts);
    }
#endif

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_match(hm->uri, mg_str("/api/stats"), NULL)) {
            // Print some statistics about currently established connections
            mg_printf(c, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_http_printf_chunk(c, "ID PROTO TYPE      LOCAL           REMOTE\n");
            for (struct mg_connection *t = c->mgr->conns; t != NULL; t = t->next) {
                mg_http_printf_chunk(c, "%-3lu %4s %s %M %M\n", t->id, t->is_udp ? "UDP" : "TCP",
                                     t->is_listening  ? "LISTENING"
                                     : t->is_accepted ? "ACCEPTED "
                                                      : "CONNECTED",
                                     mg_print_ip, &t->loc, mg_print_ip, &t->rem);
            }
            mg_http_printf_chunk(c, "");  // Don't forget the last empty chunk
        } else if (mg_match(hm->uri, mg_str("/api/f2/*"), NULL)) {
            mg_http_reply(c, 200, "", "{\"result\": \"%.*s\"}\n", (int)hm->uri.len, hm->uri.buf);
        } else {
            struct mg_http_serve_opts opts = {.root_dir = m_info.workpath};
            mg_http_serve_dir(c, (struct mg_http_message *)ev_data, &opts);
        }
    }

    return;
}

void CWebServer::InitMgLog() {
    if (!m_mgloginit) {
        m_mgloginit = true;
        mg_log_set(MG_LL_DEBUG);  // Set log level
    }

    return;
}

bool CWebServer::Init(zc_webs_info_t *info) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }
    InitMgLog();
    if (info) {
        memcpy(&m_info, info, sizeof(zc_webs_info_t));
    } else {
        memcpy(&m_info, &g_websinfodeftab[m_type], sizeof(zc_webs_info_t));
    }

    if (m_info.local) {
        // local
        snprintf(m_addrs, sizeof(m_addrs) - 1, "%s127.0.0.1:%d", g_addrprefix[m_type], m_info.port);
    } else {
        snprintf(m_addrs, sizeof(m_addrs) - 1, "%s0.0.0.0:%d", g_addrprefix[m_type], m_info.port);
    }

    m_init = true;
    LOG_TRACE("Init ok type:%d, %s, %s", m_type, m_addrs, m_info.workpath);
    return true;
}  // namespace zc

bool CWebServer::UnInit() {
    if (!m_init) {
        return true;
    }

    Stop();

    m_init = false;
    return false;
}

bool CWebServer::Start() {
    if (!m_init || m_running) {
        return false;
    }

    struct mg_mgr *mgr = new struct mg_mgr();
    if (!mgr) {
        ZC_ASSERT(0);
        LOG_ERROR("new mg_mgr error");
        return false;
    }
    // Initialise event manager
    mg_mgr_init(mgr);

    // Create HTTP listen
    if (mg_http_listen(mgr, m_addrs, EventHandlerCb, this) == nullptr) {
        LOG_ERROR("mg http listen error");
        goto _err;
    }

    m_mgrhandle = mgr;
    Thread::Start();
    LOG_TRACE("mg http listen start ok %s", m_addrs);
    return true;
_err:
    ZC_SAFE_DELETE(mgr);
    return false;
}

bool CWebServer::Stop() {
    if (!m_running) {
        return false;
    }

    Thread::Stop();
    struct mg_mgr *mgr = reinterpret_cast<struct mg_mgr *>(m_mgrhandle);
    if (mgr) {
        mg_mgr_free(mgr);
        delete mgr;
        m_mgrhandle = nullptr;
    }
    m_running = false;
    return true;
}

int CWebServer::process() {
    LOG_WARN("process into\n");
    struct mg_mgr *mgr = reinterpret_cast<struct mg_mgr *>(m_mgrhandle);
    while (State() == Running /*&&  i < loopcnt*/) {
        mg_mgr_poll(mgr, 1000);
    }
    LOG_WARN("process exit\n");
    return -1;
}

//
void CWebServerHttp::EventHandler(struct mg_connection *c, int ev, void *ev_data) {
    CWebServer::EventHandler(c, ev, ev_data);
    return;
}

void CWebServerHttps::EventHandler(struct mg_connection *c, int ev, void *ev_data) {
    CWebServer::EventHandler(c, ev, ev_data);
    return;
}

void CWebServerWS::EventHandler(struct mg_connection *c, int ev, void *ev_data) {
    CWebServer::EventHandler(c, ev, ev_data);
    return;
}

void CWebServerWSS::EventHandler(struct mg_connection *c, int ev, void *ev_data) {
    CWebServer::EventHandler(c, ev, ev_data);
    return;
}
}  // namespace zc
