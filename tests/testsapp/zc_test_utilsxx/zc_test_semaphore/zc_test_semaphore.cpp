// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// 测试信号量

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#include <memory>
#include <string>

#include "zc_log.h"

#include "Semaphore.hpp"
#include "Thread.hpp"
#include "ZcType.hpp"
#include "zc_test_semaphore.hpp"

static zc::CSem *g_semput = nullptr;
static zc::CSem *g_semget = nullptr;

class ThreadTestPut : public zc::Thread {
 public:
    explicit ThreadTestPut(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadTestPut() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        if (g_semput->Wait(100)) {
            m_process_cnt++;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            LOG_INFO("ThreadTestPut process into[%s] [%d] ", m_name.c_str(), m_process_cnt);
            g_semget->Post();
        }

        return 0;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

class ThreadTestGet : public zc::Thread {
 public:
    explicit ThreadTestGet(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~ThreadTestGet() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        if (g_semget->Wait(100)) {
            m_process_cnt++;
            // std::this_thread::sleep_for(std::chrono::milliseconds(300));
            LOG_INFO("ThreadTestGet process into[%s] [%d] ", m_name.c_str(), m_process_cnt);
            g_semput->Post();
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
    // LOG_INFO("SEM_VALUE_MAX: %ld", sysconf(_SC_SEM_VALUE_MAX));

    g_semput = new zc::CSem("semput", true);
    g_semget = new zc::CSem("semget", true);
    ZC_ASSERT(g_semput != nullptr);
    ZC_ASSERT(g_semget != nullptr);
    g_semput->Open(1);
    g_semget->Open(0);
    {
        ThreadTestPut theadA("threadA");
        ThreadTestGet theadB("threadB");
        theadB.Start();
        sleep(2);
        theadA.Start();
        while (cnt-- > 0) {
            sleep(1);
        }
        theadA.Stop();
        theadB.Stop();
    }
    g_semput->Close();
    g_semget->Close();
    ZC_SAFE_DELETE(g_semput);
    ZC_SAFE_DELETE(g_semget);
    sleep(1);
    LOG_INFO("test exit\n");

    return 0;
}



// c语言测试有名信号量
int zc_test_semaphore_c() {
    // wsl2不允许携带路径/ errno 22
    //  默认生成路径/dev/shm/sem.test1
    sem_t *sem = sem_open("test1", O_CREAT | O_EXCL, 0644, 0);
    // m_sem = sem_open(m_name.c_str(), 0);

    if (sem == SEM_FAILED) {
        LOG_ERROR("sem_open error errno[%d],", errno);
        return -1;
    }
    LOG_INFO("test ok sem[%p]\n", sem);
    sem_close(sem);
    sem_unlink("test1");
    return 0;
}

int zc_test_semaphore() {
    LOG_INFO("test into\n");
    test_thread();
    // zc_test_semaphore_c();
    LOG_INFO("test exit\n");

    return 0;
}
