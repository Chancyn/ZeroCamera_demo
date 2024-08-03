// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo.move to userspace, lockfree fifo,
// no locking version just use 1 producer 1 consumer

// shm fifo

#pragma once
#include <pthread.h>
#include <unistd.h>

#include <mutex>

#include "NonCopyable.hpp"
#include "zc_log.h"

namespace zc {
// fifo buf
typedef struct _zcshmbuf_ {
    pthread_mutex_t mutex; /* process shared mutex*/
    pthread_mutexattr_t mutexattr;
    unsigned int size;       /* the size of the allocated buffer */
    unsigned int in;         /* data is added at offset (in % size) */
    unsigned int out;        /* data is extracted from off. (out % size) */
    unsigned int key;        /* key frame data at offset */
    unsigned int latest;     /* user data update flag */
    unsigned int usersetflag;/* user data update flag */
    unsigned int rsv[1];     /* reserve */
    unsigned char buffer[0]; /* must be end the buffer holding the data */
} zcshmbuf_t;

// shmbuff == userbuf | zcshmbuf_t | fifobuffer
typedef struct _zcshmfifo_ {
    void *shmbuff;             // shmbuff ptr
    unsigned int usersize;     // userbuf size
    zcshmbuf_t *fifo;          // fifo info
    int shmid;                 // shmid
    int evfd;                  // for evevnt
    unsigned int out;
} zcshmfifo_t;
// 性能说明 ThreadPutLock ret[1024000000],cos[108-120]ms;性能与c 语言版本一致
class CShmFIFO : public NonCopyable {
 public:
    CShmFIFO(unsigned int size, const char *name, unsigned char chn, bool bwrite, unsigned int usersize = 0);
    virtual ~CShmFIFO();
    bool ShmAlloc();
    void ShmFree();

 protected:
    void setLatestpos(bool key);
    unsigned int getLatestPos(bool key);
    void setLatestOutpos(unsigned int pos);

    // no put event
    unsigned int _put(const unsigned char *buffer, unsigned int len);
    unsigned int _get(unsigned char *buffer, unsigned int len);
    void _putev();
    //
    unsigned int put(const unsigned char *buffer, unsigned int len);
    unsigned int get(unsigned char *buffer, unsigned int len);

    // userbuff get/set
    unsigned int getUserData(unsigned char *buffer, unsigned int len);
    unsigned int setUserData(unsigned char *buffer, unsigned int len);

    int ShareLock();
    int ShareUnlock();

 private:
    int share_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *mutexattr);
    int share_mutex_destroy(pthread_mutex_t *mutex, pthread_mutexattr_t *mutexattr);
    inline int share_mutex_lock(pthread_mutex_t *mutex);
    inline int share_mutex_unlock(pthread_mutex_t *mutex);

 public:
    // nolcok version unsafe be careful use, stop read/write and reset it
    void Reset();
    unsigned int Len();
    // - returns true if the fifo is empty
    bool IsEmpty();
    // - returns true if the fifo is full
    bool IsFull();
    int GetEvFd() {
        if (!m_pfifo.fifo) {
            return -1;
        }
        return m_pfifo.evfd;
    }

    int CloseEvFd() {
        if (m_pfifo.evfd > 0) {
            LOG_WARN("close evfd[%d]", m_pfifo.evfd);
            close(m_pfifo.evfd);
            m_pfifo.evfd = -1;
        }
        return 0;
    }

 private:
    bool _shmalloc(unsigned int size, int shmkey, bool bwrite);
    void _shmfree();
    int _getkey(const char *name, unsigned char chn);

 private:
    zcshmfifo_t m_pfifo;
    // pthread_mutex_t m_lock;
    // std::mutex m_mutex;
    unsigned int m_usersize;
    unsigned int m_size;
    int m_shmkey;
    const bool m_bwrite;  // read/write flag
    char m_szfifoname[128];
    char m_szevname[128];
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
}  // namespace zc
