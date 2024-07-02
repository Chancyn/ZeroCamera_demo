// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <errno.h>
#include <nng/nng.h>
#include <stddef.h>
#include <string.h>

// #include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>

#include "zc_log.h"
#include "zc_macros.h"

#include "MsgCommSubClient.hpp"
#include "ZcType.hpp"

#define ZC_DEBUG_DUMP 1  // debug dump
#if ZC_DEBUG_DUMP
#include "zc_basic_fun.h"
#endif

#define ZC_NNGREPMSG_SIZE (4096)   // TODO(zhoucc):
#define ZC_NNGREPMSG_RECVFLAG (0)  // block , NNG_FLAG_ALLOC|NNG_FLAG_NONBLOCK

namespace zc {

CMsgCommSubClient::CMsgCommSubClient() : m_psock(new nng_socket()), m_handle(nullptr), m_running(0), m_tid(0) {}

CMsgCommSubClient::~CMsgCommSubClient() {
    Close();
    if (m_psock) {
        nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
        delete psock;
        m_psock = nullptr;
    }
}

bool CMsgCommSubClient::Open(const char *url, MsgCommSubCliHandleCb handle) {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id) {
        LOG_WARN("already sock open");
        return false;
    }
    nng_socket nngsock = {};
    int rv = 0;
    if ((rv = nng_sub0_open(&nngsock)) != 0) {
        LOG_ERROR("nng_socket %d %s", rv, nng_strerror(rv));
        return false;
    }

    // subscribe to everything (empty means all topics)
    if ((rv = nng_socket_set(nngsock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) != 0) {
        LOG_ERROR("nng_socket_set %d %s", rv, nng_strerror(rv));
        goto _err_close;
    }

    if ((rv = nng_dial(nngsock, url, NULL, 0)) != 0) {
        LOG_ERROR("nng_dial %d %s", rv, nng_strerror(rv));
        goto _err_close;
    }

    m_handle = handle;
    memcpy(m_psock, &nngsock, sizeof(nng_socket));
    // LOG_TRACE("open m_sock[%d] ok", nngsock.id);
    return true;
_err_close:
    nng_close(nngsock);

    return false;
}

bool CMsgCommSubClient::Close() {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        // LOG_TRACE("close sock[%d]", psock->id);
        nng_close(*psock);
    }

    return true;
}

void *CMsgCommSubClient::runThread(void *p) {
    static_cast<CMsgCommSubClient *>(p)->runThreadProc();
    return nullptr;
}

void CMsgCommSubClient::runThreadProc() {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    LOG_INFO("subcli run into sock[%p], id[%d]", psock, psock->id);
    int rv;
    char rbuf[ZC_NNGREPMSG_SIZE] = {0};
    size_t rlen = sizeof(rbuf);

    while (m_running) {
        rlen = sizeof(rbuf);
        if ((rv = nng_recv(*psock, rbuf, &rlen, ZC_NNGREPMSG_RECVFLAG)) != 0) {
            LOG_ERROR("subcli recv msg error %d %s", rv, nng_strerror(rv));
            goto _done_err;
        }
#if ZC_DUMP_BINSTREAM
        zc_debug_dump_binstream("subcli recv", psock->id, (const uint8_t *)rbuf, rlen);
#endif
        // handle
        if (m_handle) {
            m_handle(rbuf, static_cast<int>(rlen));
        }
    }
_done_err:

    LOG_INFO("run exit sock[%p][%d]", psock, psock->id);
    return;
}

bool CMsgCommSubClient::Start() {
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

bool CMsgCommSubClient::Stop() {
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
