// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <memory>
#include <mutex>
#include <algorithm>
#include <functional>

#include "Observer.hpp"
#include "Subject.hpp"

CSubject::CSubject() {}

CSubject::~CSubject() {
    m_objObservers.clear();
}

bool CSubject::AddObserver(CObserver &observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_objObservers.push_back(&observer);

    return true;
}

bool CSubject::RemoveObserver(CObserver &observer) {
    bool bRet = false;
    std::lock_guard<std::mutex> lock(m_mutex);
    // std::list<CObserver *>::iterator iter = m_objObservers.begin();
    auto iter = m_objObservers.begin();
    for (; iter != m_objObservers.end();) {
        if (*iter == &observer) {
            m_objObservers.erase(iter++);
            bRet = true;
        } else {
            ++iter;
        }
    }

    return bRet;
}

void CSubject::NotifyObservers(ZC_U32 u32MsgId, ZC_U32 u32SubMsgId, void *pData, ZC_U32 u32DataLen) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // std::list<CObserver *>::iterator iter = m_objObservers.begin();
    auto iter = m_objObservers.begin();
    for (; iter != m_objObservers.end(); iter++) {
        (*iter)->Notify(u32MsgId, u32SubMsgId, pData, u32DataLen);
    }
    return;
}

Subject::Subject() {}

Subject::~Subject() {
    m_objObservers.clear();
}

void Subject::AddObserver(const std::shared_ptr<CObserver> &observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_objObservers.push_back(observer);

    return;
}

#if 1
void Subject::RemoveObserver(const std::shared_ptr<CObserver> &observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // std::list<std::shared_ptr<CObserver>>::iterator iter = m_objObservers.begin();
    auto iter = m_objObservers.begin();
    for (; iter != m_objObservers.end();) {
        if (iter->lock() == observer) {
            m_objObservers.erase(iter++);
        } else {
            ++iter;
        }
    }

    return;
}
#else
void Subject::RemoveObserver(std::shared_ptr<CObserver> &observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_objObservers.erase(
        std::remove_if(m_objObservers.begin(), m_objObservers.end(),
                        [&observer](std::weak_ptr<CObserver> &wp) { return wp.lock() == observer; }),
        m_objObservers.end());
}
#endif

void Subject::NotifyObservers(ZC_U32 u32MsgId, ZC_U32 u32SubMsgId, void *pData, ZC_U32 u32DataLen) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto &wp : m_objObservers) {
        if (auto observer = wp.lock()) {  // check vaild
            observer->Notify(u32MsgId, u32SubMsgId, pData, u32DataLen);
        }
    }

    return;
}
