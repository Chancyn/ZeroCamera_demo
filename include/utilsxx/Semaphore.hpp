// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// posix semaphore

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <string>

#include "zc_log.h"
#include "zc_macros.h"

#include "NonCopyable.hpp"
#include "zc_type.h"

#pragma once

namespace zc {

class CSem : public NonCopyable {
 public:
    explicit CSem(const char *name, bool bcreate = true) : m_sem(nullptr), m_name(name), m_create(bcreate) {}
    virtual ~CSem() { Close(); }
    bool Open(int value = 0) {
        if (m_sem) {
            return false;
        }

        if (m_create) {
            m_sem = sem_open(m_name.c_str(), O_CREAT | O_EXCL, 0644, value);
        } else {
            m_sem = sem_open(m_name.c_str(), 0);
        }

        if (m_sem == SEM_FAILED) {
            LOG_ERROR("sem_open error errno[%d], [%s],m_create[%d]", errno, m_name.c_str(), m_create);
            m_sem = nullptr;
            ZC_ASSERT(0);
            return false;
        }

        return true;
    }

    void Close() {
        if (m_sem) {
            if (sem_close(m_sem) == -1) {
                LOG_ERROR("sem_close error errno[%d]", errno);
                ZC_ASSERT(0);  // need to handle EBUSY
            }
            m_sem = nullptr;
            if (m_create) {
                sem_unlink(m_name.c_str());
            }
        }
    }

    bool Wait(int msec = -1) {
        ZC_ASSERT(m_sem != nullptr);
        int res;
        struct timespec ts;
        if (msec > 0) {
            res = clock_gettime(CLOCK_MONOTONIC, &ts);
            ZC_ASSERT(res != -1);
            int64_t inc = (int64_t)msec * 1000000 + ts.tv_nsec;  // nanosecs
            ts.tv_sec += (time_t)(inc / 1000000);
            ts.tv_nsec = (int64_t)(inc % 1000000);
        }
        do {
            if (msec < 0)
                res = sem_wait(m_sem);
            else if (msec == 0)
                res = sem_trywait(m_sem);
            else
                res = sem_timedwait(m_sem, &ts);
        } while (res == -1 && errno == EINTR);
        ZC_ASSERT(!res || errno == ETIMEDOUT || errno == EAGAIN);
        return !res;
    }

    void Post() {
        ZC_ASSERT(m_sem != nullptr);
        if (sem_post(m_sem) == -1) {
            LOG_ERROR("sem_post error errno[%d]", errno);
            ZC_ASSERT(0);
        }
    }

 private:
    sem_t *m_sem;
    std::string m_name;
    bool m_create;
};

// unnamed semaphore
class CUnSem : public NonCopyable {
 public:
    CUnSem() : m_sem(nullptr), m_own(false) {}

    explicit CUnSem(const CUnSem &other) {
        m_own = false;
        m_sem = (sem_t *)&other.m_holder;
    }
    virtual ~CUnSem() { Destroy(); }
    bool Init(int value = 0) {
        if (m_sem) {
            return false;
        }

        // pshared 0 share between threads
        if (sem_init(&m_holder, 0, value) == -1) {
            LOG_ERROR("sem_init error errno[%d]", errno);
            m_sem = nullptr;
            ZC_ASSERT(0);
            return false;
        }

        m_sem = &m_holder;
        return true;
    }

    void Destroy() {
        if (m_sem) {
            if (m_own && sem_destroy(m_sem) == -1) {
                LOG_ERROR("sem_destroy error errno[%d]", errno);
                ZC_ASSERT(0);  // need to handle EBUSY
            }
            m_sem = nullptr;
        }
    }

    bool Wait(int msec = -1) {
        ZC_ASSERT(m_sem != nullptr);
        if (!m_sem) {
            return false;
        }
        int res;
        struct timespec ts;
        if (msec > 0) {
            clock_gettime(CLOCK_MONOTONIC, &ts);
            int64_t inc = (int64_t)msec * 1000000 + ts.tv_nsec;  // nanosecs
            ts.tv_sec += (time_t)(inc / 1000000);
            ts.tv_nsec = (int64_t)(inc % 1000000);
        }
        do {
            if (msec < 0)
                res = sem_wait(m_sem);
            else if (msec == 0)
                res = sem_trywait(m_sem);
            else
                res = sem_timedwait(m_sem, &ts);
        } while (res == -1 && errno == EINTR);
        ZC_ASSERT(!res || errno == ETIMEDOUT || errno == EAGAIN);
        return !res;
    }

    void Post() {
        ZC_ASSERT(m_sem != nullptr);
        if (sem_post(m_sem) == -1) {
            LOG_ERROR("sem_post error errno[%d]", errno);
            ZC_ASSERT(0);
        }
    }

 private:
    sem_t m_holder;
    sem_t *m_sem;
    bool m_own;
};
}  // namespace zc
