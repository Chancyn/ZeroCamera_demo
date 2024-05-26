// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_SHMFIFO_CAPI_H__
#define __ZC_SHMFIFO_CAPI_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_frame.h"

// shmfifowrite
void *zc_shmfifow_create(unsigned int size, const char *name, unsigned char chn);
void zc_shmfifow_destroy(void *p);
int zc_shmfifow_put(void *p, const unsigned char *buffer, unsigned int len);

// shmfiforead
void *zc_shmfifor_create(unsigned int size, const char *name, unsigned char chn);
void zc_shmfifor_destroy(void *p);
int zc_shmfifor_get(void *p, unsigned char *buffer, unsigned int len);

// shmstreamwrite
void *zc_shmstreamw_create(unsigned int size, const char *name, unsigned char chn, stream_puting_cb puting_cb, void *u);
void zc_shmstreamw_destroy(void *p);
unsigned int zc_shmstreamw_put(void *p, const unsigned char *buffer, unsigned int len, void *stream);
unsigned int zc_shmstreamw_put_appending(void *p, const unsigned char *buffer, unsigned int len);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_SHMFIFO_CAPI_H__*/
