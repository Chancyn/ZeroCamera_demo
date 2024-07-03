// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <pthread.h>
#include <stddef.h>

#include <functional>

#include "zc_type.h"

typedef std::function<ZC_S32(char *data, int size)> MsgCommSubCliHandleCb;

namespace zc {
class CMsgCommSubClient {
 public:
    CMsgCommSubClient();
    ~CMsgCommSubClient();

 public:
    bool Open(const char *url, MsgCommSubCliHandleCb handle);
    bool Close();
    bool Start();
    bool Stop();

 private:
    static void *runThread(void *p);
    void runThreadProc();

 private:
    void *m_psock;  // nng_socket m_sock;
    MsgCommSubCliHandleCb m_handle;
    int m_running;
    pthread_t m_tid;
};
}  // namespace zc
