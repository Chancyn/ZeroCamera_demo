// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <pthread.h>

#include <functional>

#include "zc_type.h"

namespace zc {

// typedef int (*MsgCommReqSerHandleCb)(char *in, size_t iqsize, char *out, size_t *opsize);
typedef std::function<ZC_S32(char *in, int iqsize, char *out, int *opsize)> MsgCommReqSerHandleCb;

class CMsgCommRepServer {
 public:
    CMsgCommRepServer();
    ~CMsgCommRepServer();

 public:
    bool Open(const char *url, MsgCommReqSerHandleCb handle);
    bool Close();
    bool Start();
    bool Stop();

 private:
    static void *runThread(void *p);
    void runThreadProc();

 private:
    void *m_psock;  // nng_socket m_sock;
    MsgCommReqSerHandleCb m_handle;
    int m_running;
    pthread_t m_tid;
};
}  // namespace zc
