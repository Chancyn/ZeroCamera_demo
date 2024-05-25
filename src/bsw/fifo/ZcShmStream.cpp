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

unsigned int CShmStreamW::Put(const unsigned char *buffer, unsigned int len, bool end, void *stream) {
    unsigned int ret = 0;
    if (m_puting_cb && !end) {
        ret = _put(buffer, len);
        ret += m_puting_cb(m_u, stream);
        // callback function need call puting to put framedata append

    } else {
        // put last data, write evfifo
        ret = put(buffer, len);
    }

    return ret;
}

// donot lock, puting_cb,callback function need call PutAppending to put framedata append
unsigned int CShmStreamW::PutAppending(const unsigned char *buffer, unsigned int len, bool end) {
    if (!end) {
        return _put(buffer, len);
    } else {
        // put last data, write evfifo
        return put(buffer, len);
    }
}

CShmStreamR::CShmStreamR(unsigned int size, const char *name, unsigned char chn) : CShmFIFO(size, name, chn, false) {}

CShmStreamR::~CShmStreamR() {}

unsigned int CShmStreamR::Get(unsigned char *buffer, unsigned int len) {
    return get(buffer, len);
}

}  // namespace zc
