// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// svr
#include <mutex>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/syslog.h>

#include "srt/srt.h"
#include "zc_error.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_praseutils.h"
#include "zc_stringutils.h"

#include "Thread.hpp"
#include "ZcSrtListener.hpp"
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

CSrtListener::CSrtListener() : Thread("csrtlistener") {}
CSrtListener::~CSrtListener() {

}
int CSrtListener::GetSockOpt(int socket, SRT_SOCKOPT optname, const char *optnamestr, void *optval, int *optlen) {
    if (srt_getsockopt(socket, 0, optname, optval, optlen) < 0) {
        LOG_ERROR("failed to get option %s on socket: %s", optnamestr, srt_getlasterror_str());
        return ZCERROR(EIO);
    }
    return 0;
}

int CSrtListener::SetSockOpt(int socket, SRT_SOCKOPT optname, const char *optnamestr, const void *optval, int optlen) {
    if (srt_setsockopt(socket, 0, optname, optval, optlen) < 0) {
        LOG_ERROR("failed to set option %s on socket: %s", optnamestr, srt_getlasterror_str());
        return ZCERROR(EIO);
    }
    return 0;
}

int CSrtListener::SetNonBlock(int socket, int enable) {
    int ret, blocking = enable ? 0 : 1;
    /* Setting SRTO_{SND,RCV}SYN options to 1 enable blocking mode, setting them to 0 enable non-blocking mode. */
    ret = srt_setsockopt(socket, 0, SRTO_SNDSYN, &blocking, sizeof(blocking));
    if (ret < 0)
        return ret;
    return srt_setsockopt(socket, 0, SRTO_RCVSYN, &blocking, sizeof(blocking));
}

int CSrtListener::EpollCreate(int write) {
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

int CSrtListener::networkWait(int write) {
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

int CSrtListener::networkWaitTimeout(int write, int64_t timeout) {
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
int CSrtListener::SetOptionsPost(int fd) {
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
int CSrtListener::SetOptionsPre(int fd) {
    SRTContext *ctx = m_srtcontext;
    int connect_timeout = ctx->connect_timeout;

    if (ctx->linger >= 0) {
        struct linger lin;
        lin.l_linger = ctx->linger;
        lin.l_onoff = lin.l_linger > 0 ? 1 : 0;
        if (SetSockOpt(fd, SRTO_LINGER, "SRTO_LINGER", &lin, sizeof(lin)) < 0)
            return ZCERROR(EIO);
    }
    return 0;
}

int CSrtListener::urlSplit(const char *uri, zc_srt_url_t *pstUrl) {
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

int CSrtListener::praseUrl(const char *uri, zc_srt_url_t *pstUrl) {
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
    return 0;
err:
    return ret;
}

int CSrtListener::startListen() {
    int ret = 0;
    int os_errno;
    int reuse = 1;
    /* Max streamid length plus an extra space for the terminating null character */
    char streamid[513];
    char portstr[10];
    int streamid_len = sizeof(streamid);
    int epmodes = SRT_EPOLL_ERR | SRT_EPOLL_IN;
    struct addrinfo hints = {0}, *ai, *cur_ai;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags |= AI_PASSIVE;
    snprintf(portstr, sizeof(portstr), "%d", m_url.port);
    ret = getaddrinfo(m_url.hostname[0] ? m_url.hostname : NULL, portstr, &hints, &ai);
    if (ret) {
        LOG_ERROR("Failed to resolve hostname %s: %s", m_url.hostname, gai_strerror(ret));
        return ZCERROR(EIO);
    }

    cur_ai = ai;
    m_srtcontext->fd = srt_create_socket();
    if (m_srtcontext->fd < 0) {
        ret = zc_srt_neterrno();
        LOG_ERROR("socket error, ret:%d", ret);
        goto _err;
    }

    // set Options
    if ((ret = SetOptionsPre(m_srtcontext->fd)) < 0) {
        LOG_ERROR("error, SetOptionsPre, fd:%d, ret = %d", m_srtcontext->fd, ret);
        goto _err;
    }

    if (srt_setsockopt(m_srtcontext->fd, SOL_SOCKET, SRTO_REUSEADDR, &reuse, sizeof(reuse))) {
        LOG_WARN("setsockopt(SRTO_REUSEADDR) failed");
    }

    /* Set the socket's send or receive buffer sizes, if specified.
       If unspecified or setting fails, system default is used. */
    if (m_srtcontext->recv_buffer_size > 0) {
        srt_setsockopt(m_srtcontext->fd, SOL_SOCKET, SRTO_UDP_RCVBUF, &m_srtcontext->recv_buffer_size,
                       sizeof(m_srtcontext->recv_buffer_size));
    }

    if (m_srtcontext->send_buffer_size > 0) {
        srt_setsockopt(m_srtcontext->fd, SOL_SOCKET, SRTO_UDP_SNDBUF, &m_srtcontext->send_buffer_size,
                       sizeof(m_srtcontext->send_buffer_size));
    }

    if (SetNonBlock(m_srtcontext->fd, 1) < 0) {
        LOG_WARN("zc_srt_socket_nonblock failed\n");
    }

    // epoll create
    ret = srt_epoll_create();
    if (ret < 0) {
        ret = zc_srt_neterrno();
        goto _err;
    }
    m_srtcontext->eid = ret;

    // add epoll
    if (srt_epoll_add_usock(m_srtcontext->eid, m_srtcontext->fd, &epmodes) < 0) {
        ret = zc_srt_neterrno();
        LOG_ERROR("srt_epoll_add_usock error ret:%d", ret);
        goto _err;
    }

    if (srt_bind(m_srtcontext->fd, ai->ai_addr, ai->ai_addrlen)) {
        ret = zc_srt_neterrno();
        LOG_ERROR("srt_bind error ret:%d", ret);
        goto _err;
    }

    ret = srt_listen(m_srtcontext->fd, 1);
    if (ret < 0) {
        ret = zc_srt_neterrno();
        LOG_ERROR("srt_listen error ret:%d", ret);
        goto _err;
    }

    ret = networkWaitTimeout(0, m_timeout);
    if (ret < 0) {
        LOG_ERROR("srt_listen error ret:%d", ret);
        goto _err;
    }

    m_open = 1;
    freeaddrinfo(ai);
    return 0;
_err:
    m_open = 0;
    srt_epoll_release(m_srtcontext->eid);
    m_srtcontext->eid = 0;
    srt_close(m_srtcontext->fd);
    m_srtcontext->fd = 0;
    freeaddrinfo(ai);
    return ret;
}

int CSrtListener::stopListen() {
    if (!m_open) {
        return -1;
    }

    SRTContext *ctx = m_srtcontext;
    if (ctx->fd > 0) {
        srt_epoll_release(ctx->eid);
        ctx->eid = 0;
        srt_close(ctx->fd);
        ctx->fd = 0;
    }

    return 0;
}

int CSrtListener::Init(const char *uri, int flags) {
    if (m_init) {
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
    Thread::Start();
    m_init = 1;
    LOG_INFO("Init ok fd:%d", m_srtcontext->fd);
    return 0;
err:
    ZC_SAFE_DELETE(m_srtcontext);
    srt_cleanup();
    m_init = 0;
    return ret;
}

int CSrtListener::Read(char *buf, int size) {
    ZC_ASSERT(buf != nullptr);
    if (!m_init) {
        return -1;
    }
    int ret;
    SRTContext *ctx = m_srtcontext;
    if (!(m_flags.flags & ZC_AVIO_FLAG_NONBLOCK)) {
        ret = networkWaitTimeout(0, ctx->rw_timeout);
        if (ret)
            return ret;
    }

    ret = srt_recvmsg(ctx->fd, buf, size);
    if (ret < 0) {
        ret = zc_srt_neterrno();
    }

    return ret;
}

int CSrtListener::Write(const char *buf, int size) {
    if (!m_init) {
        return -1;
    }
    int ret;
    SRTContext *ctx = m_srtcontext;
    if (!(m_flags.flags & ZC_AVIO_FLAG_NONBLOCK)) {
        ret = networkWaitTimeout(1, ctx->rw_timeout);
        if (ret)
            return ret;
    }

    ret = srt_sendmsg(ctx->fd, buf, size, -1, 1);
    if (ret < 0) {
        ret = zc_srt_neterrno();
    }

    return ret;
}

int CSrtListener::UnInit() {
    if (m_init) {
        SRTContext *ctx = m_srtcontext;
        LOG_TRACE("srt UnInit Into:%d", ctx->fd);
        Thread::Stop();
        srt_epoll_release(ctx->eid);
        srt_close(ctx->fd);

        srt_cleanup();
        m_init = 0;
        LOG_TRACE("srt UnInit Exit");
    }
    return 0;
}

int CSrtListener::workaccept() {
    int ret = 0;
    int fd = 0;
    char clihost[NI_MAXHOST];
    char cliserv[NI_MAXSERV];
    int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
    char streamid[513];
    int streamid_len = sizeof(streamid);
    zc_srtlistener_cli_t *cli = nullptr;
    SRTContext *ctx = m_srtcontext;
    struct sockaddr_storage cliaddr;
    int cliaddrlen = 0;
    ret = srt_accept(ctx->fd, (struct sockaddr *)&cliaddr, &cliaddrlen);
    if (ret < 0) {
        ret = zc_srt_neterrno();
        return ret;
    }

    fd = ret;
    if (SetNonBlock(ret, 1) < 0) {
        LOG_ERROR("zc_srt_socket_nonblock failed");
    }

    if (!GetSockOpt(ret, SRTO_STREAMID, "SRTO_STREAMID", streamid, &streamid_len)) {
        /* Note: returned streamid_len doesn't count the terminating null character */
        LOG_ERROR("accept streamid [%s], length %d", streamid, streamid_len);
    }

    getnameinfo((struct sockaddr *)&cliaddr, cliaddrlen, clihost, sizeof(clihost), cliserv, sizeof(cliserv),
                NI_NUMERICHOST | NI_NUMERICSERV);
    LOG_INFO("new connection: fd:%d, clihost:%s, cliserv:%s", fd, clihost, cliserv);
    if (SRT_ERROR == srt_epoll_add_usock(ctx->eid, fd, &events)) {
        ret = zc_srt_neterrno();
        LOG_ERROR("accept streamid [%s], length %d", streamid, streamid_len);
        goto _err;
    }

    cli = new zc_srtlistener_cli_t();
    cli->fd = fd;
    strncpy(cli->clihost, clihost, sizeof(cli->clihost));
    strncpy(cli->cliserv, cliserv, sizeof(cli->cliserv));
    addCli(cli);
    return 0;
_err:
    if (fd > 0) {
        srt_close(fd);
    }
    return ret;
}

// add list
int CSrtListener::addCli(zc_srtlistener_cli_t *cli) {
    std::lock_guard<std::mutex> locker(m_clilistmutex);
    m_clilist.push_back(cli);
    return 0;
}

int CSrtListener::deleteCli(int fd) {
    std::lock_guard<std::mutex> locker(m_clilistmutex);
    auto iter = m_clilist.begin();
    for (; iter != m_clilist.end();) {
        if ((*iter)->fd == fd) {
            LOG_WARN("delete find cli fd:%p clihost:%s, cliserv:%s", fd, (*iter), (*iter)->clihost, (*iter)->cliserv);
            ZC_SAFE_DELETE((*iter));
            iter = m_clilist.erase(iter);
        } else {
            ++iter;
        }
    }

    return 0;
}

int CSrtListener::work() {
    // start
    int ret = 0;
    if (!startListen()) {
        LOG_ERROR("_startconn error");
        return -1;
    }

    while (State() == Running) {
        workaccept();
    }

    // stop
    stopListen();
    return ret;
}

int CSrtListener::process() {
    LOG_WARN("process into");
    while (State() == Running) {
        ZC_MSLEEP(1000);
    }
    LOG_WARN("process exit");
    return -1;
}

}  // namespace zc
