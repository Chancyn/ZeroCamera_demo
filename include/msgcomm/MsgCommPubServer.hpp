// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <pthread.h>

#include <functional>
#include <mutex>

#include "zc_type.h"

namespace zc {

class CMsgCommPubServer {
 public:
    CMsgCommPubServer();
    ~CMsgCommPubServer();

 public:
    bool Open(const char *url);
    bool Close();
    bool Send(void *buf, size_t len);
 private:
    void *m_psock;  // nng_socket m_sock;
    std::mutex m_mutex;
};
}  // namespace zc
