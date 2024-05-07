// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo.move to userspace, lockfree fifo,
// no locking version just use 1 producer 1 consumer

// copy from linux kernel(2.6.32) zcfifo.move to userspace, lockfree fifo

#pragma once
#include <pthread.h>

#include <mutex>

#include "NonCopyable.hpp"

namespace zc {
// fifo buf
typedef struct _zcshmbuf_ {
    unsigned int size;       /* the size of the allocated buffer */
    unsigned int in;         /* data is added at offset (in % size) */
    unsigned int out;        /* data is extracted from off. (out % size) */
    unsigned char buffer[0]; /* the buffer holding the data */
} zcshmbuf_t;

//
typedef struct _zcshmfifo_ {
    zcshmbuf_t *fifo;
    int shmid;   // shmid
    int evfifo;  // for evevnt
    unsigned int out;
} zcshmfifo_t;
// 性能说明 ThreadPutLock ret[1024000000],cos[108-120]ms;性能与c 语言版本一致
class CShmFIFO : public NonCopyable {
 public:
    CShmFIFO(unsigned int size, const char *name, unsigned char chn, bool bwrite);
    virtual ~CShmFIFO();
    bool ShmAlloc();
    void ShmFree();

 protected:
    unsigned int put(const unsigned char *buffer, unsigned int len);
    unsigned int get(unsigned char *buffer, unsigned int len);

 public:
    // nolcok version unsafe be careful use, stop read/write and reset it
    void Reset();
    unsigned int Len();
    // - returns true if the fifo is empty
    bool IsEmpty();
    // - returns true if the fifo is full
    bool IsFull();

 private:
    bool _shmalloc(unsigned int size, int shmkey, bool bwrite);
    void _shmfree();
    int _getkey(const char *name, unsigned char chn);

 private:
    zcshmfifo_t m_pfifo;
    pthread_mutex_t m_lock;
    unsigned int m_size;
    int m_shmkey;
    bool m_bwrite;  // read/write flag
};

class CShmFIFOW : public CShmFIFO {
 public:
    CShmFIFOW(unsigned int size, const char *name, unsigned char chn) : CShmFIFO(size, name, chn, true) {}
    virtual ~CShmFIFOW() {}
    unsigned int Put(const unsigned char *buffer, unsigned int len) { return put(buffer, len); }
    unsigned int Get(unsigned char *buffer, unsigned int len) { return get(buffer, len); }
};

class CShmFIFOR : public CShmFIFO {
 public:
    CShmFIFOR(unsigned int size, const char *name, unsigned char chn) : CShmFIFO(size, name, chn, false) {}
    virtual ~CShmFIFOR() {}

    unsigned int Get(unsigned char *buffer, unsigned int len) { return get(buffer, len); }
};
#if 0
// mutex SAFE
// 性能说明 ThreadPutLock ret[1024000000],cos[630-680]ms;std::lock_guard性能略差于c 语言版本pthread_mutex_lock(588-620)
class CFIFOSafe : public CShmFIFO {
 public:
    explicit CFIFOSafe(unsigned int size) : CShmFIFO(size) {}
    virtual ~CFIFOSafe() {}

    virtual unsigned int Put(const unsigned char *buffer, unsigned int len) {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CShmFIFO::Put(buffer, len);
    }

    virtual unsigned int Get(unsigned char *buffer, unsigned int len) {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CShmFIFO::Get(buffer, len);
    }

    virtual unsigned int Len() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CShmFIFO::Len();
    }

    // - returns true if the fifo is empty
    virtual bool IsEmpty() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CShmFIFO::IsEmpty();
    }
    // - returns true if the fifo is full
    virtual bool IsFull() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CShmFIFO::IsFull();
    }

    virtual void Reset() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return CShmFIFO::Reset();
    }

    // lockfree interface
    unsigned int Putlockfree(const unsigned char *buffer, unsigned int len) { return CShmFIFO::Put(buffer, len); }
    unsigned int Getlockfree(unsigned char *buffer, unsigned int len) { return CShmFIFO::Get(buffer, len); }
    void Resetlockfree() { return CShmFIFO::Reset(); }
    unsigned int Lenlockfree() { return CShmFIFO::Len(); }
    bool IsEmptylockfree() { return CShmFIFO::IsEmpty(); }
    bool IsFulllockfree() { return CShmFIFO::IsFull(); }

 private:
    std::mutex m_mutex;
};
#endif
}  // namespace zc
