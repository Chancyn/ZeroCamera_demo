// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <errno.h>
#include <nng/nng.h>
#include <pthread.h>
#include <string.h>

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "zc_log.h"
#include "zc_macros.h"

#include "MsgCommRepServer.hpp"
#include "ZcType.hpp"

#define ZC_DEBUG_DUMP 0    // debug dump
#if ZC_DEBUG_DUMP
#include "zc_basic_fun.h"
#endif

namespace zc {
#define ZC_NNGREPMSG_SIZE (4096)   // TODO(zhoucc):
#define ZC_NNGREPMSG_RECVFLAG (0)  // block , NNG_FLAG_ALLOC|NNG_FLAG_NONBLOCK
#define ZC_NNGREPMSG_SENDFLAG (0)

CMsgCommRepServer::CMsgCommRepServer() : m_psock(new nng_socket()), m_handle(nullptr), m_running(0), m_tid(0) {}

CMsgCommRepServer::~CMsgCommRepServer() {
    Stop();
    Close();
    if (m_psock) {
        nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
        delete psock;
        m_psock = nullptr;
    }
}

bool CMsgCommRepServer::Open(const char *url, MsgCommReqSerHandleCb handle) {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id) {
        LOG_WARN("already sock open");
        return false;
    }
    nng_socket nngsock = {};
    int rv = 0;
    if ((rv = nng_rep0_open(&nngsock)) != 0) {
        LOG_WARN("nng_rep0_open %d %s", rv, nng_strerror(rv));
        return false;
    }

    if ((rv = nng_listen(nngsock, url, NULL, 0)) != 0) {
        LOG_WARN("nng_listen %d %s", rv, nng_strerror(rv));
        goto _err_close;
    }

    m_handle = handle;
    memcpy(m_psock, &nngsock, sizeof(nng_socket));
    LOG_TRACE("open sock[%d] ok", nngsock.id);
    return true;
_err_close:
    nng_close(nngsock);

    return false;
}

bool CMsgCommRepServer::Close() {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        LOG_TRACE("close sock[%d]", psock->id);
        nng_close(*psock);
    }

    return true;
}

void *CMsgCommRepServer::runThread(void *p) {
    static_cast<CMsgCommRepServer *>(p)->runThreadProc();
    return nullptr;
}

void CMsgCommRepServer::runThreadProc() {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    LOG_INFO("run into sock[%p], id[%d]", psock, psock->id);
    int rv;
    char rbuf[ZC_NNGREPMSG_SIZE] = {0};
    char sbuf[ZC_NNGREPMSG_SIZE] = {0};
    size_t rlen = sizeof(rbuf);
    int slen = 0;

    while (m_running) {
        rlen = sizeof(rbuf);
        if ((rv = nng_recv(*psock, rbuf, &rlen, ZC_NNGREPMSG_RECVFLAG)) != 0) {
            LOG_ERROR("recv msg error %d %s", rv, nng_strerror(rv));
            goto _done_err;
        }
#if ZC_DUMP_BINSTREAM
        zc_debug_dump_binstream("svrrecv", psock->id, (const uint8_t *)rbuf, rlen);
#endif
        // handle
        if (m_handle) {
            slen = sizeof(sbuf);
            m_handle(rbuf, static_cast<int>(rlen), sbuf, &slen);
        }

        if ((rv = nng_send(*psock, sbuf, slen, ZC_NNGREPMSG_SENDFLAG)) != 0) {
            LOG_ERROR("send msg error %d %s", rv, nng_strerror(rv));
            goto _done_err;
        }
#if ZC_DUMP_BINSTREAM
        zc_debug_dump_binstream("svrsend", psock->id, (const uint8_t *)sbuf, slen);
#endif
    }
_done_err:

    LOG_INFO("run exit sock[%p][%d]", psock, psock->id);
    return;
}

bool CMsgCommRepServer::Start() {
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id == 0) {
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

bool CMsgCommRepServer::Stop() {
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
