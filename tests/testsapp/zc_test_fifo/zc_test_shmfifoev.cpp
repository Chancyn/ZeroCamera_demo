// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Epoll.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zcfifo.h"
#include "zcfifo_safe.h"

#include "Thread.hpp"
#include "ZcShmFIFO.hpp"
#include "ZcType.hpp"

#define TEST_FIFO_CXX 1  // cxx test

#define FIFO_TEST_LOOPCNT 1000
#define FIFO_TEST_BUFFLEN 1024
#define FIFO_TEST_FIFOSIZE (1024 * 1024)
#define FIFO_TEST_SHM_PATH "video"
#define FIFO_TEST_SHM_CHN 1

ZC_UNUSED static zcfifo_safe_t *g_fifo = nullptr;
static ZC_U32 g_loopcnt = FIFO_TEST_LOOPCNT;
static ZC_U8 g_buffer[FIFO_TEST_BUFFLEN] = {};
static zc::CShmFIFOR *g_cxxfifor = nullptr;
static zc::CShmFIFOW *g_cxxfifow = nullptr;

class ThreadPutLockev : public zc::Thread {
 public:
    explicit ThreadPutLockev(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadPutLockev() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
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
        unsigned int i = 0;
        while (State() == Running /*&&  i < loopcnt*/) {
#if TEST_FIFO_CXX  // cxx test
            if (1 /*!g_cxxfifow->IsFull()*/) {
                ret += g_cxxfifow->Put(buffer, sizeof(buffer));
                usleep(500*1000);
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
        LOG_INFO("ThreadPutLockev cnt[%u]errcnt[%u],ret[%u],cos[%llu]ms", loopcnt, errcnt, ret, endts - startts);
        return -1;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

class ThreadGetLockev : public zc::Thread {
 public:
    explicit ThreadGetLockev(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadGetLockev() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ZC_U64 startts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        ZC_U64 endts = 0;
        ZC_U8 buffer[FIFO_TEST_BUFFLEN];
        int ret = 0;
        unsigned int retcnt = 0;
        int cmp = 0;
        unsigned int errcnt = 0;
        unsigned int loopcnt = g_loopcnt;
        char buf[2048];
        const struct epoll_event * events = nullptr;
        zc::CEpoll ep;

        if (!ep.Create()) {
            LOG_ERROR("epoll create error");
            return -1;
        }
        ep.Add(g_cxxfifor->GetEvFd(), EPOLLIN, (void *)g_cxxfifor);
        while (State() == Running /* && i < loopcnt*/) {
            ret = ep.Wait();
            if (ret == -1) {
                LOG_ERROR("epoll wait error");
                return -1;
            } else if (ret > 0) {
                events = ep.Events();
                for (unsigned int i = 0; (int)i < ret; i++) {
                    // LOG_WARN("epoll wait ret[%d] events[%d] [%d] [%p]", ret, events[i].events, ep[i].data.fd, ep[i].data.ptr);
                    zc::CShmFIFOR *fifor = (zc::CShmFIFOR *)ep[i].data.ptr;
                     if (events[i].events & EPOLLIN) {
                        LOG_TRACE("epoll wait ok ret[%d], fd[%d] [%p], ptr[%p]", ret, ep[i].data.fd, g_cxxfifor,
                                  fifor);
                        if (read(fifor->GetEvFd(), buf, sizeof(buf)) <= 0) {
                            LOG_ERROR("epoll wait ok but read error ret[%d], fd[%d]", ret, ep[i].data.fd);
                            fifor->CloseEvFd();
                            return -1;
                        }

                        LOG_TRACE("epoll ok fifor[%p], fd[%d]", fifor,fifor->GetEvFd());
                        if (!fifor->IsEmpty()) {
                            ret = fifor->Get(buffer, sizeof(buffer));
                            retcnt += ret;
                            cmp = memcmp(g_buffer, buffer, sizeof(g_buffer));
                            if (cmp != 0) {
                                LOG_ERROR("ThreadGetLockev data different cmp[%d], ret[%u],%u", cmp, ret, retcnt);
                            }
                            ZC_ASSERT(cmp == 0);
                            i++;
                        } else {
                            errcnt++;
                            // usleep(5*1000);
                            // LOG_WARN("fifo empty, i[%u], errcnt[%u]", i, errcnt);
                        }
                    }
                }
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &ts);
        endts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        LOG_INFO("ThreadGetLockev cnt[%u]errcnt[%u],ret[%u],cos[%llu]ms", loopcnt, errcnt, retcnt, endts - startts);
        return -1;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

static ThreadGetLockev *g_threadget = nullptr;
static ThreadPutLockev *g_threadput = nullptr;

static int _zc_test_shmfifoev_start(int type) {
    for (unsigned int i = 0; i < sizeof(g_buffer); i++) {
        g_buffer[i] = i % 255;
    }

    if (type == 1) {
        g_cxxfifow = new zc::CShmFIFOW(FIFO_TEST_FIFOSIZE, FIFO_TEST_SHM_PATH, FIFO_TEST_SHM_CHN);
        ZC_ASSERT(g_cxxfifow != nullptr);
        g_cxxfifow->ShmAlloc();
    } else if (type == 0) {
        g_cxxfifor = new zc::CShmFIFOR(FIFO_TEST_FIFOSIZE, FIFO_TEST_SHM_PATH, FIFO_TEST_SHM_CHN);
        ZC_ASSERT(g_cxxfifor != nullptr);
        g_cxxfifor->ShmAlloc();
    } else {
        g_cxxfifow = new zc::CShmFIFOW(FIFO_TEST_FIFOSIZE, FIFO_TEST_SHM_PATH, FIFO_TEST_SHM_CHN);
        ZC_ASSERT(g_cxxfifow != nullptr);
        g_cxxfifow->ShmAlloc();
        g_cxxfifor = new zc::CShmFIFOR(FIFO_TEST_FIFOSIZE, FIFO_TEST_SHM_PATH, FIFO_TEST_SHM_CHN);
        ZC_ASSERT(g_cxxfifor != nullptr);
        g_cxxfifor->ShmAlloc();
    }

    if (g_cxxfifow) {
        g_threadput = new ThreadPutLockev("fifoput");
        ZC_ASSERT(g_threadput != nullptr);
        g_threadput->Start();
    }

    if (g_cxxfifor) {
        g_threadget = new ThreadGetLockev("fifoget");
        ZC_ASSERT(g_threadget != nullptr);
        g_threadget->Start();
    }

    return 0;
}

// start
int zc_test_shmfifoev_start(int type, int cnt) {
    if (cnt != 0) {
        g_loopcnt = cnt;
    }
    LOG_INFO("test_mod fifo start type[%d] [%d] into\n", type, g_loopcnt);
    _zc_test_shmfifoev_start(type);
    LOG_INFO("test_mod fifo start type[%d] [%d] end\n", type, g_loopcnt);

    return 0;
}

int zc_test_shmfifoev_stop(int cnt) {
    ZC_SAFE_DELETE(g_threadput);
    ZC_SAFE_DELETE(g_threadget);

    ZC_SAFE_DELETE(g_cxxfifor);
    ZC_SAFE_DELETE(g_cxxfifow);

    return 0;
}
/*
### 无锁队列测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/105ms=9300M/s

[2024-05-05 19:26:39.571][info][tid 197267] [zc_test_fifo.cpp 52][process]ThreadPutLockev
cnt[1000000]errcnt[2575175],ret[1024000000],cos[105]ms [2024-05-05 19:26:39.571][info][tid 197268] [zc_test_fifo.cpp
96][process]ThreadGetLockev cnt[1000000]errcnt[124640],ret[1024000000],cos[105]ms
*/
