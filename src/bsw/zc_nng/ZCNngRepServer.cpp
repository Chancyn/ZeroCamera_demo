// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <errno.h>
#include <nng/nng.h>
#include <pthread.h>
#include <string.h>

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "ZCNngRepServer.hpp"
#include "zc_log.h"

namespace zc {
#define ZC_NNGREPMSG_SIZE (4096)
#define ZC_NNGREPMSG_RECVFLAG (0)  // NNG_FLAG_ALLOC|NNG_FLAG_NONBLOCK
#define ZC_NNGREPMSG_SENDFLAG (0)

NngRepServer::NngRepServer() : m_handle(nullptr), m_running(0), m_tid(0) {
    memset(&m_sock, 0, sizeof(m_sock));
}

NngRepServer::~NngRepServer() {
    Stop();
    Close();
}

bool NngRepServer::Open(const char *url, NngReqSerHandleCb handle) {
    if (m_sock.id != 0) {
        LOG_WARN("already m_sock[%d] open", m_sock.id);
        return false;
    }

    int rv = 0;
    if ((rv = nng_rep0_open(&m_sock)) != 0) {
        LOG_WARN("nng_rep0_open %d %s", rv, nng_strerror(rv));
        memset(&m_sock, 0, sizeof(m_sock));
        return false;
    }

    if ((rv = nng_listen(m_sock, url, NULL, 0)) != 0) {
        LOG_WARN("nng_listen %d %s", rv, nng_strerror(rv));
        goto _err_close;
    }

    m_handle = handle;
    LOG_TRACE("open m_sock[%d] ok", m_sock.id);
    return true;
_err_close:
    nng_close(m_sock);
    memset(&m_sock, 0, sizeof(m_sock));

    return false;
}

bool NngRepServer::Close() {
    if (m_sock.id != 0) {
        LOG_TRACE("close m_sock[%d]", m_sock.id);
        nng_close(m_sock);
        memset(&m_sock, 0, sizeof(m_sock));
    }

    return true;
}

void *NngRepServer::runThread(void *p) {
    static_cast<NngRepServer *>(p)->runThreadProc();
    return nullptr;
}

void NngRepServer::runThreadProc() {
    LOG_INFO("run into m_sock[%d]", m_sock.id);
    int rv;
    char rbuf[ZC_NNGREPMSG_SIZE];
    char sbuf[ZC_NNGREPMSG_SIZE];
    size_t rlen = 0;
    size_t slen = 0;
    while (m_running) {
        if ((rv = nng_recv(m_sock, rbuf, &rlen, ZC_NNGREPMSG_RECVFLAG)) != 0) {
            LOG_ERROR("recv msg error %d %s", rv, nng_strerror(rv));
            goto _done_err;
        }

        LOG_ERROR("recv msg m_sock:%d len:%d", m_sock.id, rlen);
        // handle
        if (m_handle) {
            slen = sizeof(sbuf);
            m_handle(rbuf, rlen, sbuf, &slen);
        }

        if ((rv = nng_send(m_sock, sbuf, slen, ZC_NNGREPMSG_SENDFLAG)) != 0) {
            LOG_ERROR("send msg error %d %s", rv, nng_strerror(rv));
            goto _done_err;
        }
    }
_done_err:

    LOG_INFO("run exit m_sock[%d]", m_sock);
    return;
}

bool NngRepServer::Start() {
    if (m_sock.id == 0) {
        LOG_ERROR("send error invalid socket");
        return false;
    }

    if (!m_running) {
        m_running = true;
        if (pthread_create(&m_tid, NULL, runThread, this) != 0) {
            m_tid = 0;
            m_running = false;
            LOG_ERROR("start  errorno %d(%s)!\n", errno, strerror(errno));
            return false;
        }

        return true;
    }

    return false;
}

bool NngRepServer::Stop() {
    LOG_TRACE("Stop, m_tid[%d]", m_tid);
    if (m_running) {
        m_running = false;
        Close();  // close fd
        pthread_join(m_tid, NULL);
        m_tid = 0;
    }
    LOG_TRACE("Stop");
    return true;
}
}  // namespace zc
