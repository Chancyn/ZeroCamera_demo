// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "Thread.hpp"
#include "ZcModCli.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg_sys.h"

#include "mongoose.h"

#include "ZcType.hpp"
#include "ZcWebServer.hpp"

// #define ZC_HTTP_SERVERNAME "ZeroCamera"  // ser name
#define ZC_HTTP_SERVERNAME "ZeroCamrea(zhoucc)"     // "SRS/6.0.134(Hang)"  // ser name

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

typedef enum {
    zc_wakeup_msg_httpflv = 1,  // http-flv
    zc_wakeup_msg_wsflv = 2,  // ws-flv

    zc_wakeup_msg_butt,  // max
} zc_wakeup_msg_type;

typedef struct _zc_wakeup_msg {
    uint32_t msgtype;  // zc_wakeup_msg_type http-flv
    uint32_t len;      //
    uint8_t *data;
} zc_wakeup_msg_t;

bool CWebServer::m_mgloginit = false;

// modsyscli
CWebServer::CWebServer(zc_webs_type_e type, const websvr_cb_info_t &cbinfo)
    : Thread("Webs"), m_init(false), m_running(0), m_type(type), m_mgrhandle(nullptr) {
    memset(&m_info, 0, sizeof(m_info));
    memset(m_addrs, 0, sizeof(m_addrs));
    memcpy(&m_cbinfo, &cbinfo, sizeof(websvr_cb_info_t));
    if (m_type == zc_webs_type_https || m_type == zc_webs_type_wss) {
        m_ssl = true;
    } else {
        m_ssl = false;
    }
}

CWebServer::~CWebServer() {
    UnInit();
}

#pragma pack(push)
#pragma pack(1)
typedef struct _flv_tag_hdr {
    uint8_t type;           // tagtype 1字节，0x08为音频，0x09为视频，0x12为脚本
    uint8_t data_size[3];   // 3字节，表示Tag数据部分的大小（不包括头部）
    uint8_t timestamp[3];   // 3字节，表示该Tag的时间戳（相对于第一个Tag的偏移量）
    uint8_t timestamp_ext;  // 1字节，用于扩展时间戳（高8位），通常与timestamp组合使用
    uint8_t stream_id[3];   // 1字节，通常为0
} flv_tag_hdr_t;

typedef struct _flv_file_hdr {
    uint8_t signature[3];  // 'F', 'L', 'V'
    uint8_t version;       // 通常是1
    uint8_t flags;         // 标志位，用于指示是否包含音频和视频等
    uint8_t offset[4];     // 从文件开始到第一个FLV标签的偏移量（字节）
} flv_file_hdr_t;
#pragma pack(pop)

#define FLV_TAG_LEN (sizeof(flv_tag_hdr_t))       // 11
#define FLV_HDR_TAG_LEN (sizeof(flv_file_hdr_t))  // 9

static inline void be_write_uint32(uint8_t *ptr, uint32_t val) {
    ptr[0] = (uint8_t)((val >> 24) & 0xFF);
    ptr[1] = (uint8_t)((val >> 16) & 0xFF);
    ptr[2] = (uint8_t)((val >> 8) & 0xFF);
    ptr[3] = (uint8_t)(val & 0xFF);
}

static inline void be_write_uint24(uint8_t *buffer, uint32_t value_24) {
    buffer[0] = (value_24 >> 16) & 0xFF;
    buffer[1] = (value_24 >> 8) & 0xFF;
    buffer[2] = value_24 & 0xFF;
}

static inline void packFlvTagHdr(flv_tag_hdr_t *taghdr, uint8_t type, uint32_t bytes, uint32_t timestamp) {
#if 1
    // TagType
    taghdr->type = type & 0x1F;
    be_write_uint24(taghdr->data_size, bytes);
    be_write_uint24(taghdr->timestamp, timestamp);
    taghdr->timestamp_ext = (timestamp >> 24) & 0xFF;  // Timestamp Extended
    be_write_uint24(taghdr->stream_id, 0);
#else
    uint8_t *tag = reinterpret_cast<uint8_t *>(taghdr);
    tag[0] = type & 0x1F;
    // DataSize
    tag[1] = (bytes >> 16) & 0xFF;
    tag[2] = (bytes >> 8) & 0xFF;
    tag[3] = bytes & 0xFF;

    // Timestamp
    tag[4] = (timestamp >> 16) & 0xFF;
    tag[5] = (timestamp >> 8) & 0xFF;
    tag[6] = (timestamp >> 0) & 0xFF;
    tag[7] = (timestamp >> 24) & 0xFF;  // Timestamp Extended

    // StreamID(Always 0)
    tag[8] = 0;
    tag[9] = 0;
    tag[10] = 0;
#endif
    return;
}

static inline void packFlvFileHdr(flv_file_hdr_t *hdr, bool hasvideo, bool hasaudio) {
#if 1
    hdr->signature[0] = 'F';
    hdr->signature[1] = 'L';
    hdr->signature[2] = 'V';
    hdr->version = 0x01;  // File version
    hdr->flags = 0;       // Type flags (audio:0x04 & video:0x01)
    if (hasvideo)
        hdr->flags |= 0x01;
    if (hasaudio)
        hdr->flags |= 0x04;
    be_write_uint32(hdr->offset, 9);  // Data offset
#else
    uint8_t *buf = reinterpret_cast<uint8_t *>(hdr);
    buf[0] = 'F';  // FLV signature
    buf[1] = 'L';
    buf[2] = 'V';
    buf[3] = 0x01;                // File version
    buf[4] = 0x01 | 0x04;         // Type flags (audio:0x04 & video:0x01)
    be_write_uint32(buf + 5, 9);  // Data offset
    be_write_uint32(buf + 9, 0);  // PreviousTagSize0(Always 0)
#endif

    return;
}

int CWebServer::sendFlvDataCb(void *ptr, void *sess, int type, const void *data, size_t bytes, uint32_t timestamp) {
    CWebServer *webs = reinterpret_cast<CWebServer *>(ptr);
    return webs->_sendFlvDataCb(sess, type, data, bytes, timestamp);
}

// TODO(zhoucc): maybe memory leak可能存在内存泄漏
int CWebServer::_sendFlvDataCb(void *sess, int type, const void *data, size_t bytes, uint32_t timestamp) {
    struct mg_connection *con = reinterpret_cast<struct mg_connection *>(sess);
    int ret = 0;
    // taghdr(11byte) + datalen + previous_tag_len(4byte)
    uint32_t previous_tag_len = sizeof(flv_tag_hdr_t) + bytes;
    uint32_t framelen = previous_tag_len + 4;
    uint8_t *buf = new uint8_t[framelen];  // Big structure, allocate it
    flv_tag_hdr_t *tag = reinterpret_cast<flv_tag_hdr_t *>(buf);
    // Sender side:
    zc_wakeup_msg_t framemsg = {
        .msgtype = zc_wakeup_msg_httpflv,
        .len = framelen,
        .data = buf,
    };
    // LOG_WARN("write bytes:%u, len:%u, ptr:%p, c:%p, id:%lu", bytes, framelen, buf, con, con->id);
    packFlvTagHdr(tag, type, bytes, timestamp);
    memcpy(buf + sizeof(flv_tag_hdr_t), data, bytes);
    // previous_tag_len = taghdr(11byte) + datalen
    be_write_uint32(reinterpret_cast<uint8_t *>(&previous_tag_len), previous_tag_len);
    memcpy(buf + sizeof(flv_tag_hdr_t) + bytes, &previous_tag_len, 4);
    mg_wakeup((struct mg_mgr *)m_mgrhandle, con->id, &framemsg, sizeof(framemsg));  // Send a pointer to structure
    return ret;
}

int CWebServer::_sendFlvHdr(void *sess, bool hasvideo, bool hasaudio) {
    struct mg_connection *con = reinterpret_cast<struct mg_connection *>(sess);
    int ret = 0;
    flv_file_hdr_t flvhdr;
    packFlvFileHdr(&flvhdr, hasvideo, hasaudio);
    char buf[32] = {0};

    // file hdr
    snprintf(buf, sizeof(buf) - 1, "%x\r\n", (uint32_t)sizeof(flv_file_hdr_t));
    mg_send(con, buf, strlen(buf));
    mg_send(con, &flvhdr, sizeof(flv_file_hdr_t));
    mg_send(con, "\r\n", 2);
    // previous_tag_len
    snprintf(buf, sizeof(buf) - 1, "%x\r\n", 4);
    mg_send(con, buf, strlen(buf));
    uint32_t previous_tag_len = 0;
    mg_send(con, &previous_tag_len, 4);
    mg_send(con, "\r\n", 2);

    return ret;
}

int CWebServer::unInitFlvSess() {
    std::lock_guard<std::mutex> locker(m_flvsessmutex);
    auto iter = m_flvsesslist.begin();
    for (; iter != m_flvsesslist.end();) {
        ZC_SAFE_DELETE(*iter);
        iter = m_flvsesslist.erase(iter);
    }

    return 0;
}

int CWebServer::handleCloseHttpFlvSession(struct mg_connection *c, void *ev_data) {
    std::lock_guard<std::mutex> locker(m_flvsessmutex);
    auto iter = m_flvsesslist.begin();
    for (; iter != m_flvsesslist.end();) {
        if ((*iter)->GetConnSess() == c) {
            LOG_WARN("find session c:%p -> flvsess:%p", c, (*iter));
            ZC_SAFE_DELETE(*iter);
            iter = m_flvsesslist.erase(iter);
        } else {
            ++iter;
        }
    }

    return 0;
}

int CWebServer::handleOpenHttpFlvSession(struct mg_connection *c, void *ev_data) {
    LOG_TRACE("http-flv into");
    int chn = 1;
    int type = 0;
    char httphdr[256];
    zc_flvsess_info_t info = {
        .sendflvdatacb = sendFlvDataCb,
        .context = this,
        .connsess = c,
    };

    // prase channel
    if (!m_cbinfo.getStreamInfoCb || m_cbinfo.getStreamInfoCb(m_cbinfo.MgrContext, type, chn, &info.streaminfo)) {
        LOG_ERROR("get streaminfo error");
        return -1;
    }

    // create sess
    CFlvSess *sess = new CHttpFlvSess(info);
    if (!sess) {
        LOG_ERROR("new http-flv session error");
        return -1;
    }

    if (!sess->Open()) {
        LOG_ERROR("http-flv Open error");
        goto err;
    }

    if (!sess->StartSendProcess()) {
        LOG_ERROR("StartSend error");
        goto err;
    }
    snprintf(httphdr, sizeof(httphdr) - 1,
             "HTTP/1.1 200 OK\r\n"
             "Connection: Close\r\n"
             "Content-Type: video/x-flv\r\n"
             "Server: %s\r\n"
             "Transfer-Encoding: chunked\r\n\r\n",
             ZC_HTTP_SERVERNAME);

    mg_printf(c, "%s", httphdr);
    _sendFlvHdr(c, true, false);
    // add to session
    {
        std::lock_guard<std::mutex> locker(m_flvsessmutex);
        m_flvsesslist.push_back(sess);
    }

    // c->user_data = sess;

    LOG_WARN("handleOpenHttpFlvSession ok");
    return 0;
err:
    delete sess;
    LOG_ERROR("handle http-flv error");
    return -1;
}

int CWebServer::sendWsFlvDataCb(void *ptr, void *sess, int type, const void *data, size_t bytes, uint32_t timestamp) {
    CWebServer *webs = reinterpret_cast<CWebServer *>(ptr);
    return webs->_sendWsFlvDataCb(sess, type, data, bytes, timestamp);
}

// TODO(zhoucc): maybe memory leak可能存在内存泄漏
int CWebServer::_sendWsFlvDataCb(void *sess, int type, const void *data, size_t bytes, uint32_t timestamp) {
    struct mg_connection *con = reinterpret_cast<struct mg_connection *>(sess);
    int ret = 0;
    // taghdr(11byte) + datalen + previous_tag_len(4byte)
    uint32_t previous_tag_len = sizeof(flv_tag_hdr_t) + bytes;
    uint32_t framelen = previous_tag_len + 4;
    uint8_t *buf = new uint8_t[framelen];  // Big structure, allocate it
    flv_tag_hdr_t *tag = reinterpret_cast<flv_tag_hdr_t *>(buf);
    // Sender side:
    zc_wakeup_msg_t framemsg = {
        .msgtype = zc_wakeup_msg_wsflv,
        .len = framelen,
        .data = buf,
    };
    // LOG_WARN("write bytes:%u, len:%u, ptr:%p, c:%p, id:%lu", bytes, framelen, buf, con, con->id);
    packFlvTagHdr(tag, type, bytes, timestamp);
    memcpy(buf + sizeof(flv_tag_hdr_t), data, bytes);
    // previous_tag_len = taghdr(11byte) + datalen
    be_write_uint32(reinterpret_cast<uint8_t *>(&previous_tag_len), previous_tag_len);
    memcpy(buf + sizeof(flv_tag_hdr_t) + bytes, &previous_tag_len, 4);
    mg_wakeup((struct mg_mgr *)m_mgrhandle, con->id, &framemsg, sizeof(framemsg));  // Send a pointer to structure
    return ret;
}

int CWebServer::_sendWsFlvHdr(void *sess, bool hasvideo, bool hasaudio) {
    struct mg_connection *con = reinterpret_cast<struct mg_connection *>(sess);
    int ret = 0;
    flv_file_hdr_t flvhdr;
    packFlvFileHdr(&flvhdr, hasvideo, hasaudio);
     LOG_TRACE("_sendWsFlvHdr into");
    // file hdr
    mg_ws_send(con, &flvhdr, sizeof(flv_file_hdr_t), WEBSOCKET_OP_BINARY);
    // previous_tag_len
    uint32_t previous_tag_len = 0;
    mg_ws_send(con, &previous_tag_len, 4, WEBSOCKET_OP_BINARY);

    return ret;
}

int CWebServer::handleOpenWsFlvSession(struct mg_connection *c, void *ev_data) {
    LOG_TRACE("ws-flv into");
    int chn = 1;
    int type = 0;
    char httphdr[256];
    zc_flvsess_info_t info = {
        .sendflvdatacb = sendWsFlvDataCb,
        .context = this,
        .connsess = c,
    };

    // prase channel
    if (!m_cbinfo.getStreamInfoCb || m_cbinfo.getStreamInfoCb(m_cbinfo.MgrContext, type, chn, &info.streaminfo)) {
        LOG_ERROR("get streaminfo error");
        return -1;
    }
#if 1
    // create sess
    CFlvSess *sess = new CWebSocketFlvSess(info);
    if (!sess) {
        LOG_ERROR("new ws-flv session error");
        return -1;
    }

    if (!sess->Open()) {
        LOG_ERROR("ws-flv Open error");
        goto err;
    }

    if (!sess->StartSendProcess()) {
        LOG_ERROR("StartSend error");
        goto err;
    }

    #if 0
    snprintf(httphdr, sizeof(httphdr) - 1,
             "HTTP/1.1 200 OK\r\n"
             "Connection: Close\r\n"
             "Content-Type: video/x-flv\r\n"
             "Server: %s\r\n"
             "Transfer-Encoding: chunked\r\n\r\n",
             ZC_HTTP_SERVERNAME);

    mg_printf(c, "%s", httphdr);
    #endif

    _sendWsFlvHdr(c, true, false);

    // add to session
    {
        std::lock_guard<std::mutex> locker(m_flvsessmutex);
        m_flvsesslist.push_back(sess);
    }
#endif
    // c->user_data = sess;

    LOG_WARN("handleOpenWsFlvSession ok");
    return 0;
err:
    delete sess;
    LOG_ERROR("handle ws-flv error");
    return -1;
}

void CWebServer::EventHandlerCb(struct mg_connection *c, int ev, void *ev_data) {
    CWebServer *webs = reinterpret_cast<CWebServer *>(c->fn_data);
    return webs->EventHandler(c, ev, ev_data);
}

void CWebServer::EventHandler(struct mg_connection *c, int ev, void *ev_data) {
#if ZC_SUPPORT_SSL
    if (ev == MG_EV_ACCEPT && m_ssl) {
        LOG_TRACE("accept ssl");
        struct mg_tls_opts opts = {
#ifdef TLS_TWOWAY
            .ca = mg_str(s_tls_ca),
#endif
            .cert = mg_str(s_tls_cert),
            .key = mg_str(s_tls_key)};
        mg_tls_init(c, &opts);
    }
#endif
    switch (ev) {
    case MG_EV_OPEN: {
        // c->is_hexdumping = 1;
        break;
    }
    case MG_EV_CLOSE: {
        LOG_WARN("MG_EV_CLOSE session c:%p ", c);
        handleCloseHttpFlvSession(c, ev_data);
        break;
    }
    case MG_EV_WRITE: {
        // LOG_WARN("MG_EV_WRITE session c:%p ", c);
        break;
    }
    case MG_EV_WAKEUP: {
        struct mg_str *data = (struct mg_str *)ev_data;
        zc_wakeup_msg_t *framemsg = (zc_wakeup_msg_t *)data->buf;
        // LOG_WARN("MG_EV_WAKEUP len:%u, type:%u, ptr:%p, session c:%p, id:%lu", framemsg->len, framemsg->msgtype,
        // framemsg->data,
        //          c, c->id);
        if (framemsg->len > 0) {
            if (framemsg->msgtype == zc_wakeup_msg_httpflv) {
                mg_printf(c, "%x\r\n", framemsg->len);
                mg_send(c, framemsg->data, framemsg->len);
                mg_send(c, "\r\n", 2);
            } else if (framemsg->msgtype == zc_wakeup_msg_wsflv) {
                mg_ws_send(c, framemsg->data, framemsg->len, WEBSOCKET_OP_BINARY);
            }
        }
        if (framemsg->data)
            delete[] framemsg->data;
        break;
    }
    case MG_EV_HTTP_MSG: {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_match(hm->uri, mg_str("/websocket"), NULL)) {
            // Upgrade to websocket. From now on, a connection is a full-duplex
            // Websocket connection, which will receive MG_EV_WS_MSG events.
            mg_ws_upgrade(c, hm, NULL);
        } else if (mg_match(hm->uri, mg_str("/api/stats"), NULL)) {
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
        } else if (mg_match(hm->uri, mg_str("/live/*"), NULL)) {
            if (handleOpenHttpFlvSession(c, ev_data) < 0) {
                mg_http_reply(c, 404, "", "Not found\n");
            }
        } else if (mg_match(hm->uri, mg_str("/wslive/*"), NULL)) {
            mg_ws_upgrade(c, hm, NULL);
        } else if (mg_match(hm->uri, mg_str("/api/f2/*"), NULL)) {
            mg_http_reply(c, 200, "", "{\"result\": \"%.*s\"}\n", (int)hm->uri.len, hm->uri.buf);
        } else {
            struct mg_http_serve_opts opts = {.root_dir = m_info.workpath};
            mg_http_serve_dir(c, (struct mg_http_message *)ev_data, &opts);
        }
        break;
    }
    case MG_EV_WS_OPEN: {
         LOG_WARN("MG_EV_WS_OPEN session c:%p ", c);
         if (handleOpenWsFlvSession(c, ev_data) < 0) {
             mg_http_reply(c, 404, "", "Not found\n");
         }
         break;
    }
    case MG_EV_WS_CTL: {
         struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
         LOG_WARN("MG_EV_WS_CTL session c:%p len:%u, flag:%u", c, wm->data.len, wm->flags);
         break;
    }
    case MG_EV_WS_MSG: {
        // Got websocket frame. Received data is wm->data. Echo it back!
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        LOG_WARN("MG_EV_WS_CTL session c:%p len:%u, flag:%u", c, wm->data.len, wm->flags);
        // mg_ws_send(c, wm->data.buf, wm->data.len, WEBSOCKET_OP_TEXT);
        break;
    }
    default:
        break;
    }

    return;
}

void CWebServer::InitMgLog() {
    if (!m_mgloginit) {
        m_mgloginit = true;
        // mg_log_set(MG_LL_DEBUG);  // Set log level
        mg_log_set(MG_LL_INFO);  // Set log level
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

    // clear flv session
    unInitFlvSess();
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
    mg_wakeup_init(mgr);  // Initialise wakeup socket pair
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
