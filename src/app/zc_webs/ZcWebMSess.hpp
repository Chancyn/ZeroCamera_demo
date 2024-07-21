
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdlib.h>

#include "zc_frame.h"
#include "zc_type.h"

#include "NonCopyable.hpp"

#define ZC_SUPPORT_HTTP_FMP4 1  // support http-fmp4
#define ZC_SUPPORT_WS_FMP4 1    // support ws-fmp4,websocket-fmp4

namespace zc {
// type web media session
typedef enum {
    // live
    zc_web_msess_http_flv = 0,  // http-flv
    zc_web_msess_ws_flv,        // ws-flv
    zc_web_msess_http_fmp4,     // http-fmp4
    zc_web_msess_ws_fmp4,       // ws-fmp4

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
int zc_get_msess_path(char *dst, unsigned int len, zc_web_msess_type_e mtype, zc_shmstream_e type, unsigned int chn);
int zc_prase_mediasess_path(const char *url, zc_web_msess_type_e *mtype, zc_shmstream_e *type, unsigned int *chn);
}  // namespace zc
