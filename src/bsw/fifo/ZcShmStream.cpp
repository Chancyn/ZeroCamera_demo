// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// shm fifo

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "zc_frame.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"
#include "ZcType.hpp"

// userspace modify................

// userspace modify................
namespace zc {
CShmStreamW::CShmStreamW(unsigned int size, const char *name, unsigned char chn, stream_puting_cb puting_cb, void *u)
    : CShmFIFO(size, name, chn, true), m_puting_cb(puting_cb), m_u(u) {}

CShmStreamW::~CShmStreamW() {}

// put for hi_venc_stream, 1.first put hdr, 2.stream data PutAppending by m_puting_cb
unsigned int CShmStreamW::Put(const unsigned char *buffer, unsigned int len, void *stream) {
    unsigned int ret = 0;
    ShareLock();
    ret = _put(buffer, len);

    // callback function need call puting to put framedata append
    if (m_puting_cb && stream) {
        ret += m_puting_cb(m_u, stream);
    }
    // put last data, write evfifo
    _putev();
    ShareUnlock();

    return ret;
}

// donot lock, puting_cb,callback function need call PutAppending to put framedata append
unsigned int CShmStreamW::PutAppending(const unsigned char *buffer, unsigned int len) {
    return _put(buffer, len);
}

CShmStreamR::CShmStreamR(unsigned int size, const char *name, unsigned char chn) : CShmFIFO(size, name, chn, false) {}

CShmStreamR::~CShmStreamR() {}

// put for hi_venc_stream, 1.first put hdr, 2.stream data GetAppending by m_puting_cb
unsigned int CShmStreamR::Get(unsigned char *buffer, unsigned int buflen, unsigned int hdrlen, unsigned int magic) {
    unsigned int framelen = 0;
    unsigned int ret = 0;
    ShareLock();
    ret = _get(buffer, hdrlen);
    ZC_ASSERT(ret == hdrlen);
    unsigned int *magicb = (unsigned int *)buffer;
    // zc_frame_t *frame = (zc_frame_t *)buffer;
    //  framelen = frame->size;
    //  if (frame->magic != magic) {
    //      LOG_ERROR("magic[0x%x]size[%u]pts[%u]utc[%u]", frame->magic, frame->size, frame->pts, frame->utc);
    //  }
    ZC_ASSERT((*magicb) == magic);
    framelen = *(++magicb);
    // LOG_ERROR("buflen[%u]hdrlen[%u]framelen[%u]ret[%u]", buflen, hdrlen, framelen, ret);
    ZC_ASSERT(buflen >= hdrlen + framelen);
    ret = _get(buffer + hdrlen, framelen);
    ZC_ASSERT(ret == framelen);
    ShareUnlock();

    return ret + hdrlen;
}

}  // namespace zc
