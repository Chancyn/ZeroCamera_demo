// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_log.h"
#include "zc_macros.h"
#include "zc_shmfifo_capi.h"

#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"
#include "ZcType.hpp"

void *zc_shmfifow_create(unsigned int size, const char *name, unsigned char chn) {
    zc::CShmFIFOW *fifow = new zc::CShmFIFOW(size, name, chn);
    if (!fifow->ShmAlloc()) {
        LOG_ERROR("ShmAllocWrite error");
        ZC_ASSERT(0);
        return NULL;
    }

    return fifow;
}

void zc_shmfifow_destroy(void *p) {
    zc::CShmFIFOW *fifow = (zc::CShmFIFOW *)p;
    ZC_SAFE_DELETE(fifow);
    return;
}

int zc_shmfifow_put(void *p, const unsigned char *buffer, unsigned int len) {
    if (p) {
        zc::CShmFIFOW *fifow = (zc::CShmFIFOW *)p;
        return fifow->Put(buffer, len);
    }

    return 0;
}

void *zc_shmfifor_create(unsigned int size, const char *name, unsigned char chn) {
    zc::CShmFIFOR *fifor = new zc::CShmFIFOR(size, name, chn);
    if (!fifor->ShmAlloc()) {
        LOG_ERROR("ShmAllocWrite error");
        ZC_ASSERT(0);
        return NULL;
    }

    return fifor;
}

void zc_shmfifor_destroy(void *p) {
    zc::CShmFIFOR *fifor = (zc::CShmFIFOR *)p;
    ZC_SAFE_DELETE(fifor);
    return;
}

int zc_shmfifor_get(void *p, unsigned char *buffer, unsigned int len) {
    if (p) {
        zc::CShmFIFOR *fifow = (zc::CShmFIFOR *)p;
        return fifow->Get(buffer, len);
    }

    return 0;
}

void *zc_shmstreamw_create(unsigned int size, const char *name, unsigned char chn, stream_puting_cb puting_cb,
                           void *u) {
    zc::CShmStreamW *fifow = new zc::CShmStreamW(size, name, chn, puting_cb, u);
    if (!fifow->ShmAlloc()) {
        LOG_ERROR("ShmAllocWrite error");
        ZC_ASSERT(0);
        return NULL;
    }

    return fifow;
}

void zc_shmstreamw_destroy(void *p) {
    zc::CShmStreamW *fifow = (zc::CShmStreamW *)p;
    ZC_SAFE_DELETE(fifow);
    return;
}

int zc_shmstreamw_put(void *p, const unsigned char *buffer, unsigned int len, bool end, void *stream) {
    if (p) {
        zc::CShmStreamW *fifow = (zc::CShmStreamW *)p;
        return fifow->Put(buffer, len, end, stream);
    }

    return 0;
}

int zc_shmstreamw_put_appending(void *p, const unsigned char *buffer, unsigned int len, bool end) {
    if (p) {
        zc::CShmStreamW *fifow = (zc::CShmStreamW *)p;
        return fifow->PutAppending(buffer, len, end);
    }

    return 0;
}