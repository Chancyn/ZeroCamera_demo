// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "Singleton.hpp"
#include "zc_timerwheel.h"

namespace zc {

class CTimerManager;
class CTimer {
 public:
    CTimer();
    ~CTimer();
    bool Start(const struct zc_timer_info *info);

    // period timer manual stop
    bool Stop();

 private:
    struct zc_timer_info m_info;
    CTimerManager *m_ptimerMng;
    void *m_ptimer;
};

// Singleton
class CTimerManager : public Singleton<CTimerManager> {
 public:
    CTimerManager();
    ~CTimerManager();
    bool Init();
    bool UnInit();
    void *AddTimer(const struct zc_timer_info *info);
    void DelTimer(void *timer);

 private:
    void *m_timerwheel;
    std::mutex m_mutex;
};

#define g_TimerManagerInstance (zc::CTimerManager::GetInstance())
};  // namespace zc