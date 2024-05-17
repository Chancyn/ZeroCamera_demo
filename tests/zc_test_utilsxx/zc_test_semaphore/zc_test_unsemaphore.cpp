// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// 测试信号量

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "zc_log.h"

#include "Semaphore.hpp"
#include "Thread.hpp"
#include "ZcType.hpp"
#include "zc_test_unsemaphore.hpp"

static zc::CUnSem *g_unsemput = nullptr;
static zc::CUnSem *g_unsemget = nullptr;

class ThreadTestPutUn : public zc::Thread {
 public:
    explicit ThreadTestPutUn(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadTestPutUn() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        if (g_unsemput->Wait(100)) {
            m_process_cnt++;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            LOG_INFO("ThreadTestPutUn process into[%s] [%d] ", m_name.c_str(), m_process_cnt);
            g_unsemget->Post();
        }

        return 0;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

class ThreadTestGetUn : public zc::Thread {
 public:
    explicit ThreadTestGetUn(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadTestGetUn() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        if (g_unsemget->Wait(100)) {
            m_process_cnt++;
            // std::this_thread::sleep_for(std::chrono::milliseconds(300));
            LOG_INFO("ThreadTestGetUn process into[%s] [%d] ", m_name.c_str(), m_process_cnt);
            g_unsemput->Post();
        }

        return 0;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

static int test_thread() {
    LOG_INFO("test into\n");
    int cnt = 5;
    g_unsemput = new zc::CUnSem();
    g_unsemget = new zc::CUnSem();
    ZC_ASSERT(g_unsemput != nullptr);
    ZC_ASSERT(g_unsemget != nullptr);
    if (!g_unsemput->Init(1)) {
        LOG_ERROR("g_unsemput init error\n");
        goto _done_err;
    }

    if (!g_unsemget->Init(0)) {
        LOG_ERROR("g_unsemget init error\n");
        goto _done_err;
    }

    {
        ThreadTestPutUn theadA("threadA");
        ThreadTestGetUn theadB("threadB");
        theadB.Start();
        sleep(2);
        theadA.Start();
        while (cnt-- > 0) {
            sleep(1);
        }
        theadA.Stop();
        theadB.Stop();
    }

_done_err:
    g_unsemput->Destroy();
    g_unsemget->Destroy();
    ZC_SAFE_DELETE(g_unsemput);
    ZC_SAFE_DELETE(g_unsemget);
    sleep(1);
    LOG_INFO("test exit\n");

    return 0;
}

#include <errno.h>
#include <semaphore.h>

// c语言测试有名信号量
int zc_test_unsemaphore_c() {
    // wsl2不允许携带路径/ errno 22
    //  默认生成路径/dev/shm/sem.test1
    sem_t *sem = nullptr;
    sem_t m_holder;
    int ret = sem_init(&m_holder, 0, 0);
    if (ret == -1) {
        LOG_ERROR("sem_open error errno[%d],", errno);
        return -1;
    }
    sem = &m_holder;
    LOG_INFO("test ok sem[%p]\n", sem);
    sem_destroy(sem);
    return 0;
}

int zc_test_unsemaphore() {
    LOG_INFO("test into\n");
    test_thread();
    // zc_test_unsemaphore_c();
    LOG_INFO("test exit\n");

    return 0;
}
