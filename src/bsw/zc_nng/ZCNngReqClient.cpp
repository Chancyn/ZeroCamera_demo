// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <string.h>
#include <errno.h>
#include <nng/nng.h>

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "zc_log.h"
#include "ZCNngReqClient.hpp"

namespace zc {

NngReqClient::NngReqClient() {
    memset(&m_sock, 0, sizeof(m_sock));
}

NngReqClient::~NngReqClient() {
    Close();
}

bool NngReqClient::Open(const char *url) {
    if (m_sock.id != 0) {
        LOG_WARN("already m_sock[%d] open", m_sock.id);
        return false;
    }

    int rv = 0;
    if ((rv = nng_req0_open(&m_sock)) != 0) {
        LOG_ERROR("nng_socket %d %s", rv, nng_strerror(rv));
        memset(&m_sock, 0, sizeof(m_sock));
        return false;
    }

    if ((rv = nng_dial(m_sock, url, NULL, 0)) != 0) {
        LOG_ERROR("nng_dial %d %s", rv, nng_strerror(rv));
        goto _err_close;
    }

    LOG_TRACE("open m_sock[%d] ok", m_sock.id);
    return true;
_err_close:
    nng_close(m_sock);
    memset(&m_sock, 0, sizeof(m_sock));

    return false;
}

bool NngReqClient::Close() {
    if (m_sock.id != 0) {
        LOG_TRACE("close m_sock[%d]", m_sock.id);
        nng_close(m_sock);
        memset(&m_sock, 0, sizeof(m_sock));
    }

    return true;
}

bool NngReqClient::Send(void *buf, size_t len, int flags) {
    if (m_sock.id != 0) {
        int rv;
        LOG_ERROR("into send msg %d %s", m_sock, buf);
        if ((rv = nng_send(m_sock, buf, len, flags)) != 0) {
            LOG_ERROR("send msg error %d %s", rv, nng_strerror(rv));
            return false;
        }

        return true;
    }

    return false;
}

bool NngReqClient::Recv(void *buf, size_t *len, int flags) {
    if (m_sock.id != 0) {
        int rv;
        if ((rv = nng_recv(m_sock, buf, len, flags)) != 0) {
            LOG_ERROR("recv msg error %d %s", rv, nng_strerror(rv));
            return false;
        }

        return true;
    }

    return false;
}

}  // namespace zc
