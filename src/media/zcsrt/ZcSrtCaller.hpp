// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/*
 * SRT - Secure, Reliable, Transport
 * Copyright (c) 2021 Haivision Systems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; If not, see <http://www.gnu.org/licenses/>
 */

#pragma once
#include <cstdint>
#include <stdint.h>

#include <mutex>

#include "zc_srt.h"

#include "Thread.hpp"

namespace zc {
#define ZC_SRT_CALLER_RECV_BUFSIZE (1316)   // 188*7 tspkg=188
typedef struct _SRTContext SRTContext;
typedef int32_t (*srtCallReadCb)(void *ptr, const uint8_t *data, uint32_t len);

typedef struct {
    srtCallReadCb onRead;
    void *ctx;
} zc_srtcaller_info_t;

class CSrtCaller : protected Thread {
 public:
    CSrtCaller();
    explicit CSrtCaller(const zc_srtcaller_info_t &cbinfo);
    virtual ~CSrtCaller();
    int Open(const char *uri);
    int Open(const char *uri, int flags);
    int Read(uint8_t *buf, int size);
    int Write(const uint8_t *buf, int size);
    int Close();
    int GetSockOpt(int socket, SRT_SOCKOPT optname, const char *optnamestr, void *optval, int *optlen);
    int SetSockOpt(int socket, SRT_SOCKOPT optname, const char *optnamestr, const void *optval, int optlen);
    int SetNonBlock(int socket, int enable);

 private:
    int _open(const char *uri, int flags);
    void safeFreeCtx(SRTContext **ppctx);
    int setup(const char *uri, int flags);
    int srtUrlSplit(const char *uri, zc_srt_url_t *stUrl);
    int praseUrl(const char *uri, zc_srt_url_t *pstUrl);
    int praseUrlArgs(const char *uriargs);
    int EpollCreate(int write);
    int networkWait(int write);
    int networkWaitTimeout(int write, int64_t timeout);
    int connectListen(const struct sockaddr *addr, socklen_t addrlen, int64_t timeout);
    int connect(zc_srt_url_t *url, int flags);
    int SetOptionsPost(int fd);
    int SetOptionsPre(int fd);
    virtual int process();

 private:
    int m_open;
    zc_srt_url_t m_url;
    zc_srtcaller_info_t m_cbinfo;
    SRTContext *m_srtcontext;
    zc_srt_flags_t m_flags;
    uint8_t m_recvpkbuf[ZC_SRT_CALLER_RECV_BUFSIZE];
};

}  // namespace zc
