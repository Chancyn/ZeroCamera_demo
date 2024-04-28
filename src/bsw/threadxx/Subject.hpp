// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <list>

#include <mutex>
#include <memory>

#include "zc_type.h"

class CObserver;

class CSubject {
 public:
    CSubject();
    virtual ~CSubject();
    bool AddObserver(CObserver &observer);
    bool RemoveObserver(CObserver &observer);
    void NotifyObservers(ZC_U32 u32MsgId, ZC_U32 u32SubMsgId, void *pData, ZC_U32 u32DataLen);

 private:
    std::list<CObserver *> m_objObservers;
    std::mutex m_mutex;
};

// shared_ptr,weak_ptr
class Subject {
 private:
    std::list<std::weak_ptr<CObserver>> m_objObservers;
    std::mutex m_mutex;

 public:
    Subject();
    ~Subject();

    void AddObserver(const std::shared_ptr<CObserver> &observer);
    void RemoveObserver(const std::shared_ptr<CObserver> &observer);
    void NotifyObservers(ZC_U32 u32MsgId, ZC_U32 u32SubMsgId, void *pData, ZC_U32 u32DataLen);
};


