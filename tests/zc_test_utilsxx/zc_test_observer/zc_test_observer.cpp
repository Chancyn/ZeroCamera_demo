// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "Observer.hpp"
#include "Subject.hpp"
#include "zc_log.h"

#include "zc_test_observer.hpp"

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

int zc_test_observer() {
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
