// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
//#include <nng/nng.h>
#include <pthread.h>
// #include <nng/protocol/reqrep0/rep.h>
// #include <nng/protocol/reqrep0/req.h>

namespace zc {

typedef int (*NngReqSerHandleCb)(char *in, size_t iqsize, char *out, size_t *opsize);
class CMsgCommRepServer {
 public:
    CMsgCommRepServer();
    ~CMsgCommRepServer();

 public:
    bool Open(const char *url, NngReqSerHandleCb handle);
    bool Close();
    bool Start();
    bool Stop();

 private:
    static void *runThread(void *p);
    void runThreadProc();

 private:
    void *m_psock;    // nng_socket m_sock;
    NngReqSerHandleCb m_handle;
    int m_running;
    pthread_t m_tid;
};
}  // namespace zc
