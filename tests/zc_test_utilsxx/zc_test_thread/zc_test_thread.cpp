// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "zc_log.h"
#include "Thread.hpp"
#include "ThreadPool.hpp"
#include "SafeQueue.hpp"
#include "zc_test_thread.hpp"

// rand sleep
static void simulate_hard_computation() {
    static int sec = 1;
    sec++;
    std::this_thread::sleep_for(std::chrono::seconds(sec % 3));
}

static void multiply(const int a, const int b) {
    simulate_hard_computation();
    int res = a * b;
    LOG_INFO("test [%d*%d]=%d\n", a, b, res);
}

#if 0
// TODO(zhoucc) debug threadpool
static int test_threadpool() {
    LOG_INFO("test into\n");
    ThreadPool pool(4);
    pool.Start();
    for (int i = 1; i <= 3; ++i) {
        for (int j = 1; j <= 10; ++j) {
            pool.AddTask(multiply, i, j);
        }
    }
    int cnt = 5;
    while (cnt-- > 0) {
        sleep(1);
    }
    pool.Stop();
    LOG_INFO("test exit\n");

    return 0;
}
#endif

// TEST ok
static int test_threadpools() {
    LOG_INFO("test into\n");
    ThreadPools pool(4);
    pool.init();
    for (int i = 1; i <= 3; ++i) {
        for (int j = 1; j <= 10; ++j) {
            pool.AddTask(multiply, i, j);
        }
    }
    int cnt = 5;
    while (cnt-- > 0) {
        sleep(1);
    }
    pool.shutdown();
    LOG_INFO("test exit\n");

    return 0;
}

class ThreadTest : public zc::Thread {
 public:
    explicit ThreadTest(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadTest() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        m_process_cnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        LOG_INFO("ThreadTest process into[%s] [%d] ", m_name.c_str(), m_process_cnt);
        return 0;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

static int test_thread() {
    LOG_INFO("test into\n");
    int cnt = 10;
    {
        ThreadTest theadA("threadA");
        theadA.Start();
        {
            ThreadTest theadB("threadB");
            theadB.Start();
            while (cnt-- > 0) {
                sleep(1);
            }
        }
        cnt = 10;
        while (cnt-- > 0) {
            sleep(1);
        }
        theadA.Stop();
    }

    cnt = 10;
    while (cnt-- > 0) {
        sleep(1);
    }
    LOG_INFO("test exit\n");

    return 0;
}

int zc_test_thread() {
    LOG_INFO("test into\n");
    test_thread();
    test_threadpools();
    LOG_INFO("test exit\n");

    return 0;
}
