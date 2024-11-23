// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_RTMP_UTILS_H__
#define __ZC_RTMP_UTILS_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    char rurl[256];
    char host[128];
    char app[128];
    char stream[128];
    uint16_t port;
} zc_rtmp_url_t;

bool zc_rtmp_prase_url(const char *inurl, zc_rtmp_url_t *rtmpurl);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif