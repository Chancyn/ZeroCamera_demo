// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <errno.h>
#include <nng/nng.h>
#include <stddef.h>
#include <string.h>

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "zc_log.h"
#include "zc_macros.h"

#include "MsgCommReqClient.hpp"
#include "ZcType.hpp"

#include "zc_basic_fun.h"

namespace zc {

CMsgCommReqClient::CMsgCommReqClient() : m_psock(new nng_socket()) {}

CMsgCommReqClient::~CMsgCommReqClient() {
    Close();
    if (m_psock) {
        nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
        delete psock;
        m_psock = nullptr;
    }
}

bool CMsgCommReqClient::Open(const char *url) {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id) {
        LOG_WARN("already sock open");
        return false;
    }
    nng_socket nngsock = {};
    int rv = 0;
    if ((rv = nng_req0_open(&nngsock)) != 0) {
        LOG_ERROR("nng_socket %d %s", rv, nng_strerror(rv));
        return false;
    }

    if ((rv = nng_dial(nngsock, url, NULL, 0)) != 0) {
        LOG_ERROR("nng_dial %d %s", rv, nng_strerror(rv));
        goto _err_close;
    }

    memcpy(m_psock, &nngsock, sizeof(nng_socket));
    // LOG_TRACE("open m_sock[%d] ok", nngsock.id);
    return true;
_err_close:
    nng_close(nngsock);

    return false;
}

bool CMsgCommReqClient::Close() {
    ZC_ASSERT(m_psock != nullptr);
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        // LOG_TRACE("close sock[%d]", psock->id);
        nng_close(*psock);
    }

    return true;
}

bool CMsgCommReqClient::Send(void *buf, size_t len, int flags) {
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        int rv;
        // LOG_TRACE("into send msg %d %s", psock->id, buf);
        if ((rv = nng_send(*psock, buf, len, flags)) != 0) {
            LOG_ERROR("send msg error %d %s", rv, nng_strerror(rv));
            return false;
        }

        return true;
    }

    return false;
}

bool CMsgCommReqClient::Recv(void *buf, size_t *len, int flags) {
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        int rv;
        if ((rv = nng_recv(*psock, buf, len, flags)) != 0) {
            LOG_ERROR("recv msg error %d %s", rv, nng_strerror(rv));
            return false;
        }

        return true;
    }

    return false;
}

// block Sendto
bool CMsgCommReqClient::SendTo(void *buf, size_t len, void *rbuf, size_t *rlen) {
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        int rv;
        #if ZC_DUMP_BINSTREAM
        zc_debug_dump_binstream("clisend", psock->id, (const uint8_t*)buf, len);
        #endif
        // LOG_TRACE("into send msg %d %s", psock->id, buf);
        if ((rv = nng_send(*psock, buf, len, 0)) != 0) {
            LOG_ERROR("send msg error %d %s", rv, nng_strerror(rv));
            return false;
        }

        if ((rv = nng_recv(*psock, rbuf, rlen, 0)) != 0) {
            LOG_ERROR("recv msg error %d %s", rv, nng_strerror(rv));
            return false;
        }
        #if ZC_DUMP_BINSTREAM
        zc_debug_dump_binstream("clirecv", psock->id, (const uint8_t*)rbuf, *rlen);
        #endif
        return true;
    }

    return false;
}

bool CMsgCommReqClient::SendToNonBlock(void *buf, size_t len, void *rbuf, size_t *rlen) {
    nng_socket *psock = reinterpret_cast<nng_socket *>(m_psock);
    if (psock->id != 0) {
        int rv;
        // LOG_TRACE("into send msg %d %s", psock->id, buf);
        if ((rv = nng_send(*psock, buf, len, NNG_FLAG_NONBLOCK)) != 0) {
            LOG_ERROR("send msg error %d %s", rv, nng_strerror(rv));
            return false;
        }

        if ((rv = nng_recv(*psock, rbuf, rlen, NNG_FLAG_NONBLOCK)) != 0) {
            LOG_ERROR("recv msg error %d %s", rv, nng_strerror(rv));
            return false;
        }

        return true;
    }

    return false;
}

}  // namespace zc
