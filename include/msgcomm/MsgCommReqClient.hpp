// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstddef>

namespace zc {
class CMsgCommReqClient {
 public:
    CMsgCommReqClient();
    ~CMsgCommReqClient();

 public:
    bool Open(const char *url);
    bool Close();
    bool Send(void *buf, size_t len, int flags);
    bool Recv(void *buf, size_t *len, int flags);

 private:
    void *m_psock;  // nng_socket m_sock;
};
}  // namespace zc
