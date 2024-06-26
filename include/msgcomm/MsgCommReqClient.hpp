// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstddef>
#include <functional>

#include "zc_type.h"

typedef std::function<ZC_S32(char *rep, int size)>MsgCommRepCliHandleCb;

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
    bool SendTo(void *buf, size_t len, void *rbuf, size_t *rlen);
    bool SendToNonBlock(void *buf, size_t len, void *rbuf, size_t *rlen);

 private:
    void *m_psock;  // nng_socket m_sock;
};
}  // namespace zc
