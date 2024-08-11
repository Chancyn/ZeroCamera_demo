
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstdint>
#include <cstdlib>

#include "zc_frame.h"
#include "zc_type.h"

#include "NonCopyable.hpp"

namespace zc {
// type web media session
typedef enum {
    // live
    zc_web_msess_http_flv = 0,  // http-flv
    zc_web_msess_ws_flv,        // ws-flv
    zc_web_msess_http_fmp4,     // http-fmp4
    zc_web_msess_ws_fmp4,       // ws-fmp4
    zc_web_msess_http_ts,       // http-ts
    zc_web_msess_ws_ts,         // ws-ts

    zc_web_msess_type_butt,  // max
} zc_web_msess_type_e;

// session status
typedef enum {
    zc_msess_err_e,         // error
    zc_msess_uninit_e = 0,  // uninit status
    zc_msess_init_e,        // uninit status
    zc_msess_sending_e,     // running

    zc_msess_butt_e,  //
} zc_msess_status_e;

typedef int (*SendFlvDataCb)(void *ptr, void *sess, int sesstype, int type, const void *data, size_t bytes,
                             uint32_t timestamp);
typedef int (*SendFmp4DataCb)(void *ptr, void *sess, int sesstype, int type, const void *data, size_t bytes,
                              uint32_t timestamp);
typedef int (*SendTsDataCb)(void *ptr, void *sess, int sesstype, const void *data, size_t m_bytes);

typedef struct _zc_flvsess_info {
    zc_stream_info_t streaminfo;
    SendFlvDataCb sendflvdatacb;
    void *context;   // context
    void *connsess;  // http session
} zc_flvsess_info_t;

typedef struct _zc_fmp4sess_info {
    zc_stream_info_t streaminfo;
    SendFmp4DataCb sendfmp4datacb;
    void *context;   // context
    void *connsess;  // http session
} zc_fmp4sess_info_t;

typedef struct _zc_tssess_info {
    zc_stream_info_t streaminfo;
    SendTsDataCb sendtsdatacb;
    void *context;   // context
    void *connsess;  // http session
} zc_tssess_info_t;

typedef struct _zc_msess_info {
    zc_web_msess_type_e type;
    union {
        zc_flvsess_info_t flvinfo;
        zc_fmp4sess_info_t fmp4info;
        zc_tssess_info_t tsinfo;
    };
} zc_msess_info_t;

// webMediaSess
class IWebMSess {
 public:
    explicit IWebMSess(zc_web_msess_type_e type) : m_type(type) {}
    virtual ~IWebMSess() {}
    virtual bool Open() = 0;
    virtual bool Close() = 0;
    virtual bool StartSendProcess() = 0;
    virtual void *GetConnSess() = 0;

 protected:
    zc_web_msess_type_e m_type;
};

class CWebMSessFac {
 public:
    CWebMSessFac() {}
    ~CWebMSessFac() {}
    static IWebMSess *CreateWebMSess(const zc_msess_info_t &info);
};

int zc_get_msess_path(char *dst, unsigned int len, zc_web_msess_type_e mtype, zc_shmstream_e type, unsigned int chn);
int zc_prase_mediasess_path(const char *url, zc_web_msess_type_e *mtype, zc_shmstream_e *type, unsigned int *chn);

}  // namespace zc
