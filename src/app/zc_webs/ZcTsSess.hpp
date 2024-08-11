
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_type.h"

#include "NonCopyable.hpp"
#include "ZcTsMuxer.hpp"

#define ZC_SUPPORT_HTTP_TS 1  // support http-ts
#define ZC_SUPPORT_WS_TS 1    // support ws-ts,websocket-ts

namespace zc {
// type
typedef enum {
    zc_tssess_type_http = 0,  // http/https
    zc_tssess_type_ws,        // ws/wss

    zc_tssess_type_butt,  // max
} zc_tssess_type_e;

// session status
typedef enum {
    zc_tssess_err_e,         // error
    zc_tssess_uninit_e = 0,  // uninit status
    zc_tssess_init_e,        // uninit status
    zc_tssess_sending_e,     // running

    zc_tssess_butt_e,  //
} zc_tssess_status_e;

typedef int (*SendTsDataCb)(void *ptr, void *sess,  const void *data, size_t m_bytes);

typedef struct _zc_tssess_info {
    zc_stream_info_t streaminfo;
    SendTsDataCb sendtsdatacb;
    void *context;   // context
    void *connsess;  // http session
} zc_tssess_info_t;

class CTsSess : public NonCopyable {
 public:
    explicit CTsSess(zc_tssess_type_e type, const zc_tssess_info_t &info);
    virtual ~CTsSess();
    bool Open();
    bool Close();
    bool StartSendProcess();
    void *GetConnSess() { return m_info.connsess; }

 private:
    static int OnTsPacketCb(void *param, const void *data, size_t bytes);
    virtual int _onTsPacketCb(const void *data, size_t bytes);

 private:
    zc_tssess_type_e m_type;
    zc_tssess_status_e m_status;  // zc_tssess_status_e
    CTsMuxer m_tsmuxer;
    zc_tssess_info_t m_info;
};

#if ZC_SUPPORT_HTTP_TS
class CHttpTsSess : public CTsSess {
 public:
    explicit CHttpTsSess(const zc_tssess_info_t &info) : CTsSess(zc_tssess_type_http, info) {}
    virtual ~CHttpTsSess() {}
};

class CWebSocketTsSess : public CTsSess {
 public:
    explicit CWebSocketTsSess(const zc_tssess_info_t &info) : CTsSess(zc_tssess_type_ws, info) {}
    virtual ~CWebSocketTsSess() {}
};
#endif

}  // namespace zc
