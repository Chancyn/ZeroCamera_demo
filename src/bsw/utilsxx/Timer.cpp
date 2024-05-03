// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// #include <stdlib.h>
#include <string.h>

#include "zc_log.h"
#include "zc_timerwheel.h"

#include "Timer.hpp"

namespace zc {

CTimer::CTimer() : m_ptimerMng(&CTimerManager::GetInstance()), m_ptimer(nullptr) {
    memset(&m_info, 0, sizeof(m_info));
    LOG_TRACE("constructor CTimer");
}

CTimer::~CTimer() {
    // period forver timer, destructor delete timer itself
    if (m_ptimer && m_info.timercnt < 0) {
        LOG_TRACE("forver auto delete timer");
        Stop();
    }
    LOG_TRACE("destructor CTimer");
}

bool CTimer::Start(const struct zc_timer_info *info) {
    m_ptimer = m_ptimerMng->AddTimer(info);
    if (m_ptimer) {
        memcpy(&m_info, info, sizeof(m_info));
        return true;
    }

    return false;
}

bool CTimer::Stop() {
    m_ptimerMng->DelTimer(m_ptimer);
    m_ptimer = nullptr;
    return true;
}

CTimerManager::CTimerManager() : m_timerwheel(nullptr) {
    LOG_TRACE("CTimerManager CTimer");
}

CTimerManager::~CTimerManager() {
    LOG_TRACE("destructor CTimerManager");
}

bool CTimerManager::Init() {
    if (m_timerwheel) {
        LOG_ERROR("already init");
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_timerwheel = zc_timerwheel_create();
    if (!m_timerwheel) {
        LOG_ERROR("create error");
        return false;
    }

    return true;
}

bool CTimerManager::UnInit() {
    if (m_timerwheel) {
        LOG_ERROR("not init");
        return false;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    zc_timerwheel_destroy(m_timerwheel);
    m_timerwheel = nullptr;

    return true;
}

void *CTimerManager::AddTimer(const struct zc_timer_info *info) {
    if (!m_timerwheel)
        return nullptr;
    std::lock_guard<std::mutex> lock(m_mutex);

    return zc_add_timer(m_timerwheel, info);
}

void CTimerManager::DelTimer(void *phandle) {
    if (!m_timerwheel)
        return;

    return zc_del_timer(phandle);
}

};  // namespace zc