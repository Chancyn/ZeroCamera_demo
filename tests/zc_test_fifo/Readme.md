## 无锁kfifo测试

## 测试fifo
# 编译zcfifo.h编译带锁版本和不带锁版本
// 定义锁类型 0无锁/1自旋锁/2互斥锁/
#define ZC_FIFO_LOCK_MUTEX 1

##测试
// 1带锁版本; 1000000执行次数
./zc_tests_mutex 1 1000000

/*spinlock版本fifo
### 无锁队列测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/105ms=9300MB/s
ThreadPut ,cos[105]ms ThreadGet ,cos[105]ms


### mutex_lock 队列测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/600ms=1627MB/s
ThreadPutLock cos[603]ms ThreadGetLock,cos[603]ms

/*spinlock版本fifo
### 测试性能事件不等
### spinlock队列测试性能 1000000 次，每次写入1024个字节耗时245~1100ms不等，速度 1024000000/200ms=3985~968MB/s
ThreadPutLock ,cos[245]ms ThreadGetLock cos[245]ms

ThreadPutLock ,cos[397]ms ThreadGetLock cos[397]ms

ThreadPutLock ,cos[1008]ms ThreadGetLock cos[1008]ms

*/

C++版本实现性能说明
// 性能说明 CFIFO C++无锁版本 ret[1024000000],cos[108-120]ms;性能与c 语言版本一致

// 性能说明 CFIFOSafe(std::lock_guard mutex锁版本) ret[1024000000],cos[630-680]ms;std::lock_guard性能略差于c 语言版本pthread_mutex_lock(588-620)



// 性能说明 ThreadPutLock ret[1024000000],cos[630-680]ms;std::lock_guard性能略差于c 语言版本pthread_mutex_lock(588-620)


### 不带锁版本测试
```
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "zc_log.h"
#include "zc_macros.h"
#include "zcfifo.h"

#include "Thread.hpp"
#include "ZcType.hpp"

#define FIFO_TEST_LOOPCNT 1000
#define FIFO_TEST_BUFFLEN 1024
#define FIFO_TEST_FIFOSIZE (1024 * 1024)

zcfifo_t *g_fifo = nullptr;
ZC_U32 g_loopcnt = FIFO_TEST_LOOPCNT;
ZC_U8 g_buffer[FIFO_TEST_BUFFLEN] = {};

class ThreadPut : public zc::Thread {
 public:
    explicit ThreadPut(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadPut() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
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
            if (!zcfifo_is_full(g_fifo)) {
                ret += zcfifo_put(g_fifo, buffer, sizeof(buffer));
                i++;
            } else {
                errcnt++;
                // usleep(5*1000);
                // LOG_WARN("fifo full, i[%u], errcnt[%u]", i, errcnt);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &ts);
        endts = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        LOG_INFO("ThreadPut cnt[%u]errcnt[%u],ret[%u],cos[%llu]ms", loopcnt, errcnt, ret, endts - startts);
        return -1;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

class ThreadGet : public zc::Thread {
 public:
    explicit ThreadGet(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadGet() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
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
            if (!zcfifo_is_empty(g_fifo)) {
                ret = zcfifo_get(g_fifo, buffer, sizeof(buffer));
                retcnt += ret;
                cmp = memcmp(g_buffer, buffer, sizeof(g_buffer));
                if (cmp != 0) {
                    LOG_ERROR("ThreadGet data different cmp[%d], ret[%u],%u", cmp, ret, retcnt);
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
        LOG_INFO("ThreadGet cnt[%u]errcnt[%u],ret[%u],cos[%llu]ms", loopcnt, errcnt, retcnt, endts - startts);
        return -1;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

ThreadGet *g_threadget = nullptr;
ThreadPut *g_threadput = nullptr;

int _zc_test_fifo_start() {
    for (unsigned int i = 0; i < sizeof(g_buffer); i++) {
        g_buffer[i] = i % 255;
    }

    g_fifo = zcfifo_alloc(FIFO_TEST_FIFOSIZE);
    ZC_ASSERT(g_fifo != NULL);
    g_threadput = new ThreadPut("fifoput");
    g_threadget = new ThreadGet("fifoget");
    ZC_ASSERT(g_threadput != nullptr);
    ZC_ASSERT(g_threadget != nullptr);
    g_threadput->Start();
    g_threadget->Start();

    return 0;
}

// start
int zc_test_fifo_start(int cnt) {
    if (cnt != 0) {
        g_loopcnt = cnt;
    }
    LOG_INFO("test_mod fifo start[%d] into\n", g_loopcnt);
    _zc_test_fifo_start();
    LOG_INFO("test_mod fifo start[%d] end\n", g_loopcnt);

    return 0;
}

int zc_test_fifo_stop(int cnt) {
    if (g_fifo) {
        zcfifo_free(g_fifo);
        g_fifo = nullptr;
    }

    ZC_SAFE_DELETE(g_threadput);
    ZC_SAFE_DELETE(g_threadget);
    return 0;
}

```
### 带锁版本测试
```
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "zc_log.h"
#include "zc_macros.h"
#include "zcfifo.h"

#include "Thread.hpp"
#include "ZcType.hpp"

#define FIFO_TEST_LOOPCNT 1000
#define FIFO_TEST_BUFFLEN 1024
#define FIFO_TEST_FIFOSIZE (1024 * 1024)

static zcfifo_t *g_fifo = nullptr;
static ZC_U32 g_loopcnt = FIFO_TEST_LOOPCNT;
static ZC_U8 g_buffer[FIFO_TEST_BUFFLEN] = {};

class ThreadPutLock : public zc::Thread {
 public:
    explicit ThreadPutLock(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
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
            if (!zcfifo_is_full(g_fifo)) {
                ret += zcfifo_safe_put(g_fifo, buffer, sizeof(buffer));
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
    unsigned int m_process_cnt;
};

class ThreadGetLock : public zc::Thread {
 public:
    explicit ThreadGetLock(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
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
            if (!zcfifo_is_empty(g_fifo)) {
                ret = zcfifo_safe_get(g_fifo, buffer, sizeof(buffer));
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

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

static ThreadGetLock *g_threadget = nullptr;
static ThreadPutLock *g_threadput = nullptr;
static zcfifo_lock_t g_lock;


static int _zc_test_fifo_start() {
    for (unsigned int i = 0; i < sizeof(g_buffer); i++) {
        g_buffer[i] = i % 255;
    }

    ZCFIFO_LOCK_INIT(&g_lock);
    // pthread_spin_init(&g_lock, PTHREAD_PROCESS_PRIVATE);

    g_fifo = zcfifo_safe_alloc(FIFO_TEST_FIFOSIZE, &g_lock);
    ZC_ASSERT(g_fifo != NULL);
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
    if (g_fifo) {
        zcfifo_safe_free(g_fifo);
        g_fifo = nullptr;
    }

    ZC_SAFE_DELETE(g_threadput);
    ZC_SAFE_DELETE(g_threadget);
    return 0;
}
/*
### 无锁队列测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/105ms=9300M/s

[2024-05-05 19:26:39.571][info][tid 197267] [zc_test_fifo.cpp 52][process]ThreadPutLock cnt[1000000]errcnt[2575175],ret[1024000000],cos[105]ms
[2024-05-05 19:26:39.571][info][tid 197268] [zc_test_fifo.cpp 96][process]ThreadGetLock cnt[1000000]errcnt[124640],ret[1024000000],cos[105]ms
*/
```