// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo.move to userspace, lockfree fifo,
// no locking version just use 1 producer 1 consumer

// copy from linux kernel(2.6.32) zcfifo.move to userspace, lockfree fifo

#pragma once
#include "NonCopyable.hpp"
#include "Semaphore.hpp"
#include "zc_log.h"
#include <mutex>

namespace zc {
typedef struct _zcfifo_ {
    unsigned char *buffer; /* the buffer holding the data */
    unsigned int size;     /* the size of the allocated buffer */
    unsigned int in;       /* data is added at offset (in % size) */
    unsigned int out;      /* data is extracted from off. (out % size) */
} zcfifo_t;

// 性能说明 ThreadPutLock ret[1024000000],cos[108-120]ms;性能与c 语言版本一致
class CFIFO : public NonCopyable {
 public:
    explicit CFIFO(unsigned int size);
    virtual ~CFIFO();

    virtual unsigned int Put(const unsigned char *buffer, unsigned int len);
    virtual unsigned int Get(unsigned char *buffer, unsigned int len);
    virtual unsigned int GetBlock(unsigned char *buffer, unsigned int len, int msec) { return Get(buffer, len); }
    // nolcok version unsafe be careful use, stop read/write and reset it
    virtual void Reset();
    virtual unsigned int Len();
    // - returns true if the fifo is empty
    virtual bool IsEmpty();
    // - returns true if the fifo is full
    virtual bool IsFull();

 private:
    bool _alloc(unsigned int size);
    void _free();

 private:
    zcfifo_t m_fifo;
};

// mutex SAFE
// 性能说明 ThreadPutLock ret[1024000000],cos[630-680]ms;std::lock_guard性能略差于c 语言版本pthread_mutex_lock(588-620)
class CFIFOSafe : public CFIFO {
 public:
    explicit CFIFOSafe(unsigned int size) : CFIFO(size), m_semput(new CUnSem()) { m_semput->Init(); }
    virtual ~CFIFOSafe() {}

    virtual unsigned int Put(const unsigned char *buffer, unsigned int len) {
        unsigned int ret = 0;
        std::lock_guard<std::mutex> locker(m_mutex);
        ret = CFIFO::Put(buffer, len);
        m_semput->Post();
        return ret;
    }

    virtual unsigned int Get(unsigned char *buffer, unsigned int len) {
        std::lock_guard<std::mutex> locker(m_mutex);

        return CFIFO::Get(buffer, len);
    }

    virtual unsigned int GetBlock(unsigned char *buffer, unsigned int len, int msec) {
        unsigned int ret = 0;
        std::lock_guard<std::mutex> locker(m_mutex);
        if (m_semput->Wait(msec)) {
            if (!CFIFO::IsEmpty()) {
                ret = CFIFO::Get(buffer, len);
            } else {
                LOG_WARN("zhoucc empty");
            }
        }

        return ret;
    }
    virtual unsigned int Len() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CFIFO::Len();
    }

    // - returns true if the fifo is empty
    virtual bool IsEmpty() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CFIFO::IsEmpty();
    }
    // - returns true if the fifo is full
    virtual bool IsFull() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CFIFO::IsFull();
    }

    virtual void Reset() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CFIFO::Reset();
    }

    // lockfree interface
    unsigned int Putlockfree(const unsigned char *buffer, unsigned int len) { return CFIFO::Put(buffer, len); }
    unsigned int Getlockfree(unsigned char *buffer, unsigned int len) { return CFIFO::Get(buffer, len); }
    void Resetlockfree() { return CFIFO::Reset(); }
    unsigned int Lenlockfree() { return CFIFO::Len(); }
    bool IsEmptylockfree() { return CFIFO::IsEmpty(); }
    bool IsFulllockfree() { return CFIFO::IsFull(); }

 private:
    std::mutex m_mutex;
    CUnSem *m_semput;
};
}  // namespace zc
