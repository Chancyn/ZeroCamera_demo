// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <errno.h>
#include <mutex>
#include <nng/nng.h>
#include <pthread.h>
#include <string.h>

#include <nng/protocol/pubsub0/pub.h>
// #include <nng/protocol/pubsub0/sub.h>

#include "zc_log.h"
#include "zc_macros.h"

#include "MsgCommPubServer.hpp"
#include "ZcType.hpp"

#define ZC_DEBUG_DUMP 1  // debug dump
#if ZC_DEBUG_DUMP
#include "zc_basic_fun.h"
#endif

namespace zc {
#define ZC_NNGREPMSG_SIZE (4096)   // TODO(zhoucc):
#define ZC_NNGREPMSG_RECVFLAG (0)  // block , NNG_FLAG_ALLOC|NNG_FLAG_NONBLOCK
#define ZC_NNGREPMSG_SENDFLAG (0)

CMsgCommPubServer::CMsgCommPubServer() : m_psock(new nng_socket()) {

}

CMsgCommPubServer::~CMsgCommPubServer() {
    Close();
    if (m_psock) {
        nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
        delete psock;
        m_psock = nullptr;
    }
}

bool CMsgCommPubServer::Open(const char *url) {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id) {
        LOG_WARN("already sock open");
        return false;
    }
    nng_socket nngsock = {};
    int rv = 0;
    if ((rv = nng_pub0_open(&nngsock)) != 0) {
        LOG_WARN("nng_pub0_open %d %s", rv, nng_strerror(rv));
        return false;
    }

    if ((rv = nng_listen(nngsock, url, NULL, 0)) != 0) {
        LOG_WARN("nng_listen %d %s", rv, nng_strerror(rv));
        goto _err_close;
    }

    memcpy(m_psock, &nngsock, sizeof(nng_socket));
    LOG_TRACE("open pubsvr:%s m_sock[%d] ok", url, nngsock.id);
    return true;
_err_close:
    nng_close(nngsock);

    return false;
}

bool CMsgCommPubServer::Close() {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        LOG_TRACE("close sock[%d]", psock->id);
        nng_close(*psock);
    }

    return true;
}

// block Sendto
bool CMsgCommPubServer::Send(void *buf, size_t len) {
    std::lock_guard<std::mutex> locker(m_mutex);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        int rv;
#if ZC_DUMP_BINSTREAM
        zc_debug_dump_binstream("pub send", psock->id, (const uint8_t *)buf, len);
#endif
        // LOG_TRACE("into send msg %d %s", psock->id, buf);
        if ((rv = nng_send(*psock, buf, len, 0)) != 0) {
            LOG_ERROR("pub, send msg error %d %s", rv, nng_strerror(rv));
            return false;
        }
        return true;
    }

    return false;
}
}  // namespace zc
