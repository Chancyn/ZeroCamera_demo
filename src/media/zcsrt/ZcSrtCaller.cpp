// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// svr
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>

#include "srt/srt.h"

#include "zc_error.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_praseutils.h"
#include "zc_stringutils.h"

#include "ZcSrtCaller.hpp"
#include "ZcType.hpp"

namespace zc {
static inline int zc_srt_neterrno() {
    int os_errno;
    int err = srt_getlasterror(&os_errno);
    if (err == SRT_EASYNCRCV || err == SRT_EASYNCSND)
        return ZCERROR(EAGAIN);
    LOG_ERROR("errno:%d, %s", err, srt_getlasterror_str());
    return os_errno ? ZCERROR(os_errno) : ZCERR_UNKNOWN;
}

CSrtCaller::CSrtCaller() : Thread("csrtcaller"), m_open(0), m_srtcontext(nullptr) {
    memset(&m_url, 0, sizeof(m_url));
    memset(&m_cbinfo, 0, sizeof(zc_srtcaller_info_t));
    memset(&m_flags, 0, sizeof(zc_srt_flags_t));
}

CSrtCaller::CSrtCaller(const zc_srtcaller_info_t &cbinfo) : Thread("csrtcaller"), m_open(0), m_srtcontext(nullptr) {
    memset(&m_url, 0, sizeof(m_url));
    memcpy(&m_cbinfo, &cbinfo, sizeof(zc_srtcaller_info_t));
    memset(&m_flags, 0, sizeof(zc_srt_flags_t));
}

CSrtCaller::~CSrtCaller() {}
int CSrtCaller::GetSockOpt(int socket, SRT_SOCKOPT optname, const char *optnamestr, void *optval, int *optlen) {
    if (srt_getsockopt(socket, 0, optname, optval, optlen) < 0) {
        LOG_ERROR("failed to get option %s on socket: %s", optnamestr, srt_getlasterror_str());
        return ZCERROR(EIO);
    }
    return 0;
}

int CSrtCaller::SetSockOpt(int socket, SRT_SOCKOPT optname, const char *optnamestr, const void *optval, int optlen) {
    if (srt_setsockopt(socket, 0, optname, optval, optlen) < 0) {
        LOG_ERROR("failed to set option %s on socket: %s", optnamestr, srt_getlasterror_str());
        return ZCERROR(EIO);
    }
    return 0;
}

int CSrtCaller::SetNonBlock(int socket, int enable) {
    int ret, blocking = enable ? 0 : 1;
    /* Setting SRTO_{SND,RCV}SYN options to 1 enable blocking mode, setting them to 0 enable non-blocking mode. */
    ret = srt_setsockopt(socket, 0, SRTO_SNDSYN, &blocking, sizeof(blocking));
    if (ret < 0)
        return ret;
    return srt_setsockopt(socket, 0, SRTO_RCVSYN, &blocking, sizeof(blocking));
}

int CSrtCaller::EpollCreate(int write) {
    int modes = SRT_EPOLL_ERR | (write ? SRT_EPOLL_OUT : SRT_EPOLL_IN);
    int eid = srt_epoll_create();
    if (eid < 0)
        return zc_srt_neterrno();
    if (srt_epoll_add_usock(eid, m_srtcontext->fd, &modes) < 0) {
        srt_epoll_release(eid);
        return zc_srt_neterrno();
    }
    return eid;
}

int CSrtCaller::networkWait(int write) {
    int ret, len = 1, errlen = 1;
    SRTSOCKET ready[1];
    SRTSOCKET error[1];

    if (write) {
        ret = srt_epoll_wait(m_srtcontext->eid, error, &errlen, ready, &len, SRT_POLLING_TIME, 0, 0, 0, 0);
    } else {
        ret = srt_epoll_wait(m_srtcontext->eid, ready, &len, error, &errlen, SRT_POLLING_TIME, 0, 0, 0, 0);
    }
    if (ret < 0) {
        if (srt_getlasterror(NULL) == SRT_ETIMEOUT)
            ret = ZCERROR(EAGAIN);
        else
            ret = zc_srt_neterrno();
    } else {
        ret = errlen ? ZCERROR(EIO) : 0;
    }
    return ret;
}

/* TODO de-duplicate code from ff_network_wait_fd_timeout() */

int CSrtCaller::networkWaitTimeout(int write, int64_t timeout) {
    int ret;
    int64_t wait_start = 0;

    while (State() == Running) {
        ret = networkWait(write);
        if (ret != ZCERROR(EAGAIN))
            return ret;
        if (timeout > 0) {
            if (!wait_start)
                wait_start = av_gettime_relative();
            else if (av_gettime_relative() - wait_start > timeout)
                return ZCERROR(ETIMEDOUT);
        }
    }

    return 0;
}

int CSrtCaller::connectListen(const struct sockaddr *addr, socklen_t addrlen, int64_t timeout) {
    int ret;

    if (srt_connect(m_srtcontext->fd, addr, addrlen) < 0)
        return zc_srt_neterrno();

    ret = networkWaitTimeout(1, timeout);
    if (ret < 0) {
        LOG_ERROR("Connection to %s failed: %d", addr->sa_data, ret);
    }
    return ret;
}

static inline int zc_srt_setsockopt(int fd, SRT_SOCKOPT optname, const char *optnamestr, const void *optval,
                                    int optlen) {
    if (srt_setsockopt(fd, 0, optname, optval, optlen) < 0) {
        LOG_ERROR("failed to set option %s on socket: %s", optnamestr, srt_getlasterror_str());
        return ZCERROR(EIO);
    }
    return 0;
}

/* - The "POST" options can be altered any time on a connected socket.
     They MAY have also some meaning when set prior to connecting; such
     option is SRTO_RCVSYN, which makes connect/accept call asynchronous.
     Because of that this option is treated special way in this app. */
int CSrtCaller::SetOptionsPost(int fd) {
    SRTContext *ctx = m_srtcontext;

    if ((ctx->inputbw >= 0 &&
         zc_srt_setsockopt(ctx->fd, SRTO_INPUTBW, "SRTO_INPUTBW", &ctx->inputbw, sizeof(ctx->inputbw)) < 0) ||
        (ctx->oheadbw >= 0 &&
         zc_srt_setsockopt(ctx->fd, SRTO_OHEADBW, "SRTO_OHEADBW", &ctx->oheadbw, sizeof(ctx->oheadbw)) < 0)) {
        return ZCERROR(EIO);
    }
    return 0;
}

/* - The "PRE" options must be set prior to connecting and can't be altered
     on a connected socket, however if set on a listening socket, they are
     derived by accept-ed socket. */
int CSrtCaller::SetOptionsPre(int fd) {
    SRTContext *ctx = m_srtcontext;
    int connect_timeout = ctx->connect_timeout;

    if (ctx->linger >= 0) {
        struct linger lin;
        lin.l_linger = ctx->linger;
        lin.l_onoff = lin.l_linger > 0 ? 1 : 0;
        if (SetSockOpt(fd, SRTO_LINGER, "SRTO_LINGER", &lin, sizeof(lin)) < 0)
            return ZCERROR(EIO);
    }

    if (ctx->streamid) {
        if (SetSockOpt(fd, SRTO_STREAMID, "SRTO_STREAMID", ctx->streamid, strlen(ctx->streamid)) < 0) {
            return ZCERROR(EIO);
        }
    }
    LOG_TRACE("SetOptionsPre ok");
    return 0;
}

int CSrtCaller::urlSplit(const char *uri, zc_srt_url_t *pstUrl) {
    if (uri == NULL || pstUrl == NULL) {
        return -1;
    }

    int port = 0;

    zc_url_split(pstUrl->proto, sizeof(pstUrl->proto), NULL, 0, pstUrl->hostname, sizeof(pstUrl->hostname), &port,
                 pstUrl->path, sizeof(pstUrl->path), uri);
    zc_strlcpy(pstUrl->uri, uri, sizeof(pstUrl->uri));
    pstUrl->port = port;
    if (strcmp(pstUrl->proto, "srt"))
        return -1;

    if (port <= 0 || port >= 65536) {
        LOG_ERROR("Port missing in uri");
        return -1;
    }

    return 0;
}

int CSrtCaller::praseUrlArgs(const char *uriargs) {
    SRTContext *s = m_srtcontext;
    const char *p = uriargs;
    char buf[1024];
    if (zc_prase_key_value(p, "streamid", buf, sizeof(buf))) {
        ZC_SAFE_FREE(s->streamid);
        s->streamid = zc_urldecode(buf, 1);
        if (!s->streamid) {
            LOG_ERROR("zc_urldecode error");
            return -1;
        }
        LOG_TRACE("prase streamid:%s", s->streamid);
    }

    return 0;
}

int CSrtCaller::praseUrl(const char *uri, zc_srt_url_t *pstUrl) {
    const char *p;
    char opt[1024];
    int ret = 0;
    // split url
    ret = urlSplit(uri, pstUrl);
    if (ret < 0) {
        return -1;
    }

    // prase SRT options (srt/srt.h)
    p = strchr(uri, '?');
    if (p) {
        if (zc_prase_key_value(p, "mode", opt, sizeof(opt))) {
            if (!strcmp(opt, "caller")) {
                pstUrl->srtmode = SRT_MODE_CALLER;
            } else if (!strcmp(opt, "listener")) {
                pstUrl->srtmode = SRT_MODE_LISTENER;
            } else if (!strcmp(opt, "rendezvous")) {
                pstUrl->srtmode = SRT_MODE_RENDEZVOUS;
            } else {
                LOG_ERROR("mode:%s", opt);
                return -1;
            }
        }
    }

    LOG_WARN("prase: ok,uri:%s, proto:%s,hostname:%s, path:%s,port:%lu, mode:%d ", pstUrl->uri, pstUrl->proto,
             pstUrl->hostname, pstUrl->path, pstUrl->port, pstUrl->srtmode);

    if (praseUrlArgs(pstUrl->path) < 0) {
        LOG_ERROR("praseUrlArgs: error");
        return -1;
    }

    return 0;
}

int CSrtCaller::connect(zc_srt_url_t *url, int flags) {
    int ret = 0;
    SRTContext *ctx = m_srtcontext;

    struct addrinfo hints = {0}, *ai, *cur_ai;
    char portstr[10];
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags |= AI_PASSIVE;
    snprintf(portstr, sizeof(portstr), "%d", m_url.port);
    ret = getaddrinfo(url->hostname[0] ? url->hostname : NULL, portstr, &hints, &ai);

    if (ret) {
        LOG_ERROR("Failed to resolve hostname %s: %s", url->hostname, gai_strerror(ret));
        return ZCERROR(EIO);
    }
    cur_ai = ai;
    ctx->fd = srt_create_socket();
    if (ctx->fd < 0) {
        ret = zc_srt_neterrno();
        return -1;
    }

    if ((ret = SetOptionsPre(ctx->fd)) < 0) {
        LOG_ERROR("error, SetOptionsPre, fd:%d, ret = %d", ctx->fd, ret);
        goto _err;
    }

    /* Set the socket's send or receive buffer sizes, if specified.
       If unspecified or setting fails, system default is used. */
    if (ctx->recv_buffer_size > 0) {
        srt_setsockopt(ctx->fd, SOL_SOCKET, SRTO_UDP_RCVBUF, &ctx->recv_buffer_size, sizeof(ctx->recv_buffer_size));
    }
    if (ctx->send_buffer_size > 0) {
        srt_setsockopt(ctx->fd, SOL_SOCKET, SRTO_UDP_SNDBUF, &ctx->send_buffer_size, sizeof(ctx->send_buffer_size));
    }
    if (SetNonBlock(ctx->fd, 1) < 0) {
        LOG_WARN("zc_srt_socket_nonblock failed\n");
    }

    ctx->eid = EpollCreate(flags & ZC_AVIO_FLAG_WRITE);
    if (ctx->eid < 0) {
        LOG_ERROR("epoll create error:ret:%d, fd:%d", ctx->eid, ctx->fd);
        goto _err;
    }
    // connect
    connectListen(ai->ai_addr, ai->ai_addrlen, ctx->connect_timeout);
    if (flags & ZC_AVIO_FLAG_WRITE) {
        int packet_size = 0;
        int optlen = sizeof(packet_size);
        ret = GetSockOpt(ctx->fd, SRTO_PAYLOADSIZE, "SRTO_PAYLOADSIZE", &packet_size, &optlen);
        if (ret < 0)
            goto _err;
        if (packet_size > 0)
            m_flags.max_packet_size = packet_size;
    }

    m_flags.is_streamed = 1;
    LOG_INFO("srt listemn OK fd:%d, eid:%d", ctx->fd, ctx->eid);
    return 0;
_err:
    if (ctx->eid > 0) {
        srt_epoll_release(ctx->eid);
        ctx->eid = 0;
    }
    if (ctx->fd > 0) {
        srt_close(ctx->fd);
        ctx->fd = 0;
    }

    return -1;
}

void CSrtCaller::safeFreeCtx(SRTContext **ppctx) {
    if (ppctx == nullptr) {
        return;
    }

    SRTContext *pctx = *ppctx;
    if (pctx) {
        if (pctx->streamid)
            free(pctx->streamid);
        if (pctx->smoother)
            free(pctx->smoother);
    }
    *ppctx = nullptr;
    return;
}

int CSrtCaller::Open(const char *uri, int flags) {
    if (m_open) {
        LOG_ERROR("Already open");
        return -1;
    }

    int ret = 0;
    if (srt_startup() < 0) {
        LOG_ERROR("srt_startup error");
        return ZCERR_UNKNOWN;
    }

    memset(&m_url, 0, sizeof(m_url));
    m_srtcontext = new SRTContext();
    if (!m_srtcontext) {
        LOG_ERROR("new error");
        goto err;
    }
    ret = praseUrl(uri, &m_url);
    if (ret < 0) {
        LOG_ERROR("praseUrl error");
        goto err;
    }

    ret = connect(&m_url, flags);
    if (ret < 0) {
        LOG_ERROR("setup error");
        goto err;
    }

    m_open = 1;
    LOG_INFO("Open ok fd:%d", m_srtcontext->fd);
    return 0;
err:
    safeFreeCtx(&m_srtcontext);
    srt_cleanup();
    m_open = 0;
    return ret;
}

int CSrtCaller::Read(uint8_t *buf, int size) {
    ZC_ASSERT(buf != nullptr);
    if (!m_open) {
        return -1;
    }
    int ret = 0;
    SRTContext *ctx = m_srtcontext;
    if (!(m_flags.flags & ZC_AVIO_FLAG_NONBLOCK)) {
        ret = networkWaitTimeout(0, ctx->rw_timeout);
        if (ret)
            return ret;
    }

    ret = srt_recvmsg(ctx->fd, (char *)buf, size);
    if (ret < 0) {
        ret = zc_srt_neterrno();
    }

    return ret;
}

int CSrtCaller::Write(const uint8_t *buf, int size) {
    if (!m_open) {
        return -1;
    }
    int ret = 0;
    SRTContext *ctx = m_srtcontext;
    if (!(m_flags.flags & ZC_AVIO_FLAG_NONBLOCK)) {
        ret = networkWaitTimeout(1, ctx->rw_timeout);
        if (ret)
            return ret;
    }

    ret = srt_sendmsg(ctx->fd, (const char *)buf, size, -1, 1);
    if (ret < 0) {
        ret = zc_srt_neterrno();
    }

    return ret;
}

int CSrtCaller::Close() {
    if (m_open) {
        SRTContext *ctx = m_srtcontext;
        LOG_TRACE("srt close Into:%d", ctx->fd);
        srt_epoll_release(ctx->eid);
        srt_close(ctx->fd);
        safeFreeCtx(&m_srtcontext);
        srt_cleanup();
        m_open = 0;
        LOG_TRACE("srt close Exit");
    }
    return 0;
}

int CSrtCaller::process() {
    LOG_WARN("process into");
    ZC_SRT_CALLER_RECV_BUFSIZE;
    int ret = 0;
    int rbuflen = 0;
    while (State() == Running) {
        ret = Read(m_recvpkbuf, rbuflen);
        if (ret > 0) {
            if (m_cbinfo.onRead) {
                m_cbinfo.onRead(m_cbinfo.ctx, m_recvpkbuf, ret);
            } else {
                LOG_TRACE("recv ok ret:%d, rbuflen:%u", ret, rbuflen);
            }
        } else if (ret < 0) {
            if (ret != ZCERROR(ETIMEDOUT)) {
                LOG_WARN("recv error ret:%d", ret);
                break;
            }
        } else {
            LOG_WARN("recv error ret:%d", ret);
        }
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
