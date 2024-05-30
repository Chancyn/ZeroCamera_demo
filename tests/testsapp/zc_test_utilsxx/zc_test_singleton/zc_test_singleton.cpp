// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "zc_log.h"
#include "Singleton.hpp"
#include "zc_test_singleton.hpp"


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

static int test_Instance() {
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



static int test_Instancederived() {
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

int zc_test_Instance() {
    LOG_INFO("test into\n");

    test_Instance();
    test_Instancederived();
    LOG_INFO("test exit\n");

    return 0;
}
