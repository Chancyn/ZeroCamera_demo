// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "Observer.hpp"
#include "SafeQueue.hpp"
#include "Singleton.hpp"
#include "Subject.hpp"
#include "Thread.hpp"
#include "ThreadPool.hpp"

#include "zc_log.h"
#include "zc_list.h"

#include "zc_test_thread.hpp"

// rand sleep
void simulate_hard_computation() {
    static int sec = 1;
    sec++;
    std::this_thread::sleep_for(std::chrono::seconds(sec % 3));
}

void multiply(const int a, const int b) {
    simulate_hard_computation();
    int res = a * b;
    LOG_INFO("test [%d*%d]=%d\n", a, b, res);
}

// derived TestA::GetInstance()
class TestA : public Singleton<TestA> {
 public:
    void Print() { LOG_WARN("Develive TestA GetInstance,%p,this %p", &TestA::GetInstance(), this); }
};

// TestB &b = Singleton<TestB>::GetInstance();
class TestB : public Singleton<TestB> {
 public:
    TestB() { LOG_WARN("Constructor TestB ,%p", this); }
    virtual ~TestB() { LOG_WARN("Destructor TestB"); }
    void Print() { LOG_WARN("Print, TestB %p this %p", &TestB::GetInstance(), this); }
};

// TestArgsA &b = Singleton<TestArgsA>::GetInstance("test", 1);
class TestArgsA : public SingletonArgs<TestArgsA> {
 public:
    explicit TestArgsA(std::string name = "", int num = 0) : m_name(name), m_num(num) {
        LOG_WARN("Constructor ,%s, %d", name.c_str(), num);
    }
    virtual ~TestArgsA() {
        // LOG_WARN("Destructor");
        LOG_WARN("TestArgsA Destructor");
    }

 public:
    void Print() { LOG_WARN("Print TestArgsA m_name,%p,m_num[%d] this %p", &TestArgsA::GetInstance(), m_num, this); }

 private:
    std::string m_name;
    int m_num;
};

int test_Instance() {
    LOG_INFO("test into\n");

    {
        TestA::GetInstance();
        TestB::GetInstance();
        // TestB &b = Singleton<TestB>::GetInstance();
        TestArgsA &Aargs = SingletonArgs<TestArgsA>::GetInstance("testargs", 1);
        TestA::GetInstance().Print();
        TestB::GetInstance().Print();
        // b.Print();
        Aargs.Print();
        SingletonArgs<TestArgsA>::DesInstance();
    }
    return 0;
}

class SubjectTestA : public Subject {
 public:
    SubjectTestA() {}
    virtual ~SubjectTestA() {}
    void TestSubject(ZC_U32 u32MsgId, ZC_U32 u32SubMsgId) {
        char data[128] = {"subject notify test !!!"};
        snprintf(data, sizeof(data) - 1, "subject notify test u32MsgId %d, u32SubMsgId %d!!!", u32MsgId, u32SubMsgId);
        ZC_U32 u32DataLen = strlen(data) + 1;
        LOG_WARN("Notify %d, %s", u32DataLen, data);
        NotifyObservers(u32MsgId, u32SubMsgId, data, u32DataLen);
    }
};

class ObserverTest : public CObserver {
 public:
    explicit ObserverTest(std::string name) : m_name(name) { LOG_WARN("Constructor into [%s]\n", m_name.c_str()); }
    virtual ~ObserverTest() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual void Notify(ZC_U32 u32MsgId, ZC_U32 u32SubMsgId, void *pData, ZC_U32 u32DataLen) {
        LOG_INFO("Observer[%s] recv u32MsgId[%d][%d] %d, %s", m_name.c_str(), u32MsgId, u32SubMsgId, u32DataLen, pData);
    }

 private:
    std::string m_name;
};

int test_Instancederived() {
    LOG_INFO("test into\n");
    {
        TestA::GetInstance();
        TestB &b = Singleton<TestB>::GetInstance();
        TestArgsA &Aargs = SingletonArgs<TestArgsA>::GetInstance("testargs", 1);
        TestA::GetInstance().Print();
        b.Print();
        Aargs.Print();
        SingletonArgs<TestArgsA>::DesInstance();
    }
    return 0;
}

int test_subject() {
    LOG_INFO("test into\n");
    const std::shared_ptr<SubjectTestA> A = std::make_shared<SubjectTestA>();
    const std::shared_ptr<CObserver> B1 = std::make_shared<ObserverTest>("B1");

    {
        const std::shared_ptr<CObserver> B2 = std::make_shared<ObserverTest>("B2");
        A->AddObserver(B1);
        A->AddObserver(B2);

        for (unsigned int i = 0; i < 3; i++) {
            for (unsigned int j = 0; j < 3; j++) {
                A->TestSubject(i, j);
                sleep(1);
            }
        }
    }

    LOG_INFO("reset B2\n");
    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 3; j++) {
            A->TestSubject(i, j);
            sleep(1);
        }
    }

    LOG_INFO("test exit\n");
    return 0;
}

int test_threadpool() {
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
    virtual void process() {
        m_process_cnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        LOG_INFO("ThreadTest process into[%s] [%d] ", m_name.c_str(), m_process_cnt);
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

int test_thread() {
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

int test_threadxx() {
    LOG_INFO("test into\n");
    // test_threadpool();
    // test_Instance();
    // test_Instancederived();
    // test_subject();
    test_thread();
    LOG_INFO("test exit\n");

    return 0;
}
