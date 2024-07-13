
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_type.h"

#include "NonCopyable.hpp"
#include "ZcFlvMuxer.hpp"

#define ZC_SUPPORT_HTTP_FLV 1  // support http-flv
#define ZC_SUPPORT_WS_FLV 1    // support ws-flv,websocket-flv

namespace zc {
// type
typedef enum {
    zc_flvsess_type_http = 0,  // http/https
    zc_flvsess_type_ws,        // ws/wss

    zc_flvsess_type_butt,  // max
} zc_flvsess_type_e;

// session status
typedef enum {
    zc_flvsess_err_e,         // error
    zc_flvsess_uninit_e = 0,  // uninit status
    zc_flvsess_init_e,        // uninit status
    zc_flvsess_sending_e,     // running

    zc_flvsess_butt_e,  //
} zc_flvsess_status_e;

typedef int (*SendFlvDataCb)(void *ptr, void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);
typedef int (*SendFlvHdrCb)(void *ptr, void *sess, bool hasvideo, bool haseaudio);

typedef struct _zc_flvsess_info {
    zc_stream_info_t streaminfo;
    SendFlvDataCb sendflvdatacb;
    SendFlvHdrCb sendflvhdrcb;
    void *context;   // context
    void *connsess;  // http session
} zc_flvsess_info_t;

class CFlvSess : public NonCopyable {
 public:
    explicit CFlvSess(zc_flvsess_type_e type);
    virtual ~CFlvSess();
    bool Open(const zc_flvsess_info_t &info);
    bool Close();
    bool StartSendProcess();

 private:
    static int OnFlvPacketCb(void *param, int type, const void *data, size_t bytes, ZC_U32 timestamp);
    virtual int _onFlvPacketCb(int type, const void *data, size_t bytes, ZC_U32 timestamp);

 private:
    zc_flvsess_type_e m_type;
    zc_flvsess_status_e m_status;  // zc_flvsess_status_e
    CFlvMuxer m_flvmuxer;
    zc_flvsess_info_t m_info;
    bool m_bsendhdr;             // first
};

#if ZC_SUPPORT_HTTP_FLV
class CHttpFlvSess : public CFlvSess {
 public:
    CHttpFlvSess() : CFlvSess(zc_flvsess_type_http) {}
    virtual ~CHttpFlvSess() {}
};

class CWebSocketFlvSess : public CFlvSess {
 public:
    CWebSocketFlvSess() : CFlvSess(zc_flvsess_type_ws) {}
    virtual ~CWebSocketFlvSess() {}
};
#endif

}  // namespace zc
