// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include "zc_log.h"
#include "zc_macros.h"
#include "zcfifo.h"
#include "zcfifo_safe.h"

#include "Thread.hpp"
#include "ZcFIFO.hpp"
#include "ZcType.hpp"
#include "zc_test_fifo_lock.hpp"

#define TEST_FIFO_CXX 1  // cxx test

#define FIFO_TEST_LOOPCNT 1000
#define FIFO_TEST_BUFFLEN 1024
#define FIFO_TEST_FIFOSIZE (1024 * 1024)

#if !TEST_FIFO_CXX  // cxx test
ZC_UNUSED static zcfifo_safe_t *g_fifo = nullptr;
#endif

static ZC_U32 g_loopcnt = FIFO_TEST_LOOPCNT;
static ZC_U8 g_buffer[FIFO_TEST_BUFFLEN] = {};
static zc::CFIFOSafe *g_cxxfifo = nullptr;

class ThreadPutLock : public zc::Thread {
 public:
    explicit ThreadPutLock(std::string name) : Thread(name), m_name(name) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadPutLock() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ZC_U64 startts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        ZC_U64 endts = 0;
        ZC_U8 buffer[FIFO_TEST_BUFFLEN];
        memcpy(buffer, g_buffer, sizeof(g_buffer));
        unsigned int ret = 0;
        unsigned int errcnt = 0;
        unsigned int loopcnt = g_loopcnt;
        for (unsigned i = 0; i < loopcnt;) {
#if TEST_FIFO_CXX  // cxx test
            if (!g_cxxfifo->IsFull()) {
                ret += g_cxxfifo->Put(buffer, sizeof(buffer));
#else
            if (!zcfifo_safe_is_full(g_fifo)) {
                ret += zcfifo_safe_put(g_fifo, buffer, sizeof(buffer));
#endif
                i++;
            } else {
                errcnt++;
                // usleep(5*1000);
                // LOG_WARN("fifo full, i[%u], errcnt[%u]", i, errcnt);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &ts);
        endts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        LOG_INFO("ThreadPutLock cnt[%u]errcnt[%u],ret[%u],cos[%llu]ms", loopcnt, errcnt, ret, endts - startts);
        return -1;
    }

 private:
    std::string m_name;
};

class ThreadGetLock : public zc::Thread {
 public:
    explicit ThreadGetLock(std::string name) : Thread(name), m_name(name) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadGetLock() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ZC_U64 startts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        ZC_U64 endts = 0;
        ZC_U8 buffer[FIFO_TEST_BUFFLEN];
        unsigned int ret = 0;
        unsigned int retcnt = 0;
        int cmp = 0;
        unsigned int errcnt = 0;
        unsigned int loopcnt = g_loopcnt;
        for (unsigned i = 0; i < loopcnt;) {
#if TEST_FIFO_CXX  // cxx test
#if 0
            if (!g_cxxfifo->IsEmpty()) {
                ret = g_cxxfifo->Get(buffer, sizeof(buffer));
#else
            if (1 /*!g_cxxfifo->IsEmpty()*/) {
                ret = g_cxxfifo->GetBlock(buffer, sizeof(buffer), 100);
                if (ret == 0) {
                    errcnt++;
                    // LOG_WARN("fifo empty, i[%u], errcnt[%u]", i, errcnt);
                    continue;
                }

#endif
#else
            if (!zcfifo_safe_is_empty(g_fifo)) {
                ret = zcfifo_safe_get(g_fifo, buffer, sizeof(buffer));
#endif
            retcnt += ret;
            cmp = memcmp(g_buffer, buffer, sizeof(g_buffer));
            if (cmp != 0) {
                LOG_ERROR("ThreadGetLock data different cmp[%d], ret[%u],%u", cmp, ret, retcnt);
            }
            ZC_ASSERT(cmp == 0);
            i++;
        } else {
            errcnt++;
            // usleep(5*1000);
            // LOG_WARN("fifo empty, i[%u], errcnt[%u]", i, errcnt);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &ts);
    endts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    LOG_INFO("ThreadGetLock cnt[%u]errcnt[%u],ret[%u],cos[%llu]ms", loopcnt, errcnt, retcnt, endts - startts);
    return -1;
}

 private :
    std::string m_name;
};

static ThreadGetLock *g_threadget = nullptr;
static ThreadPutLock *g_threadput = nullptr;
ZC_UNUSED static zcfifo_safe_lock_t g_lock;

static int _zc_test_fifo_start() {
    for (unsigned int i = 0; i < sizeof(g_buffer); i++) {
        g_buffer[i] = i % 255;
    }

#if TEST_FIFO_CXX  // cxx test
    g_cxxfifo = new zc::CFIFOSafe(FIFO_TEST_FIFOSIZE);
    ZC_ASSERT(g_cxxfifo != nullptr);
#else
    ZCFIFO_LOCK_INIT(&g_lock);
    // pthread_spin_init(&g_lock, PTHREAD_PROCESS_PRIVATE);
    g_fifo = zcfifo_safe_alloc(FIFO_TEST_FIFOSIZE, &g_lock);
    ZC_ASSERT(g_fifo != NULL);
#endif

    g_threadput = new ThreadPutLock("fifoput");
    g_threadget = new ThreadGetLock("fifoget");
    ZC_ASSERT(g_threadput != nullptr);
    ZC_ASSERT(g_threadget != nullptr);
    g_threadput->Start();
    g_threadget->Start();

    return 0;
}

// start
int zc_test_fifo_lock_start(int cnt) {
    if (cnt != 0) {
        g_loopcnt = cnt;
    }
    LOG_INFO("test_mod fifo start[%d] into\n", g_loopcnt);
    _zc_test_fifo_start();
    LOG_INFO("test_mod fifo start[%d] end\n", g_loopcnt);

    return 0;
}

int zc_test_fifo_lock_stop(int cnt) {
    ZC_SAFE_DELETE(g_threadput);
    ZC_SAFE_DELETE(g_threadget);

#if TEST_FIFO_CXX  // cxx test
    ZC_SAFE_DELETE(g_cxxfifo);
#else
    if (g_fifo) {
        zcfifo_safe_free(g_fifo);
        g_fifo = nullptr;
    }
#endif
    return 0;
}
/*
### 无锁队列测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/105ms=9300M/s

[2024-05-05 19:26:39.571][info][tid 197267] [zc_test_fifo.cpp 52][process]ThreadPutLock
cnt[1000000]errcnt[2575175],ret[1024000000],cos[105]ms [2024-05-05 19:26:39.571][info][tid 197268] [zc_test_fifo.cpp
96][process]ThreadGetLock cnt[1000000]errcnt[124640],ret[1024000000],cos[105]ms
*/