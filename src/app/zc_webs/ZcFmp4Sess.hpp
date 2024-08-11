
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_type.h"

#include "NonCopyable.hpp"
#include "ZcFmp4Muxer.hpp"

#define ZC_SUPPORT_HTTP_FMP4 1  // support http-fmp4
#define ZC_SUPPORT_WS_FMP4 1    // support ws-fmp4,websocket-fmp4

namespace zc {
// type
typedef enum {
    zc_fmp4sess_type_http = 0,  // http/https
    zc_fmp4sess_type_ws,        // ws/wss

    zc_fmp4sess_type_butt,  // max
} zc_fmp4sess_type_e;

// session status
typedef enum {
    zc_fmp4sess_err_e,         // error
    zc_fmp4sess_uninit_e = 0,  // uninit status
    zc_fmp4sess_init_e,        // uninit status
    zc_fmp4sess_sending_e,     // running

    zc_fmp4sess_butt_e,  //
} zc_fmp4sess_status_e;

typedef int (*SendFmp4DataCb)(void *ptr, void *sess, int type, const void *data, size_t bytes, uint32_t timestamp);

typedef struct _zc_fmp4sess_info {
    zc_stream_info_t streaminfo;
    SendFmp4DataCb sendfmp4datacb;
    void *context;   // context
    void *connsess;  // http session
} zc_fmp4sess_info_t;

class CFmp4Sess : public NonCopyable {
 public:
    explicit CFmp4Sess(zc_fmp4sess_type_e type, const zc_fmp4sess_info_t &info);
    virtual ~CFmp4Sess();
    bool Open();
    bool Close();
    bool StartSendProcess();
    void *GetConnSess() { return m_info.connsess; }

 private:
    static int OnFmp4PacketCb(void *param, int type, const void *data, size_t bytes, ZC_U32 timestamp);
    virtual int _onFmp4PacketCb(int type, const void *data, size_t bytes, ZC_U32 timestamp);

 private:
    zc_fmp4sess_type_e m_type;
    zc_fmp4sess_status_e m_status;  // zc_fmp4sess_status_e
    CFmp4Muxer m_fmp4muxer;
    zc_fmp4sess_info_t m_info;
};

#if ZC_SUPPORT_HTTP_FMP4
class CHttpFmp4Sess : public CFmp4Sess {
 public:
    explicit CHttpFmp4Sess(const zc_fmp4sess_info_t &info) : CFmp4Sess(zc_fmp4sess_type_http, info) {}
    virtual ~CHttpFmp4Sess() {}
};

class CWebSocketFmp4Sess : public CFmp4Sess {
 public:
    explicit CWebSocketFmp4Sess(const zc_fmp4sess_info_t &info) : CFmp4Sess(zc_fmp4sess_type_ws, info) {}
    virtual ~CWebSocketFmp4Sess() {}
};
#endif
}  // namespace zc
