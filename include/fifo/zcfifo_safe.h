// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo_safe.move to userspace, lockfree fifo

#ifndef __ZCFIFO_SAFE_H__
#define __ZCFIFO_SAFE_H__
// copy from linux kernel(2.6.32) zcfifo_safe.move to userspace, lockfree fifo

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <pthread.h>  // for spinlock
#include <stdbool.h>

// 定义锁类型 0无锁/1自旋锁/2互斥锁/
#define ZC_FIFO_LOCK_MUTEX 2

#if (ZC_FIFO_LOCK_MUTEX == 0)
#warning "fifo no lock"
typedef int zcfifo_safe_lock_t;
#define ZCFIFO_LOCK(x)
#define ZCFIFO_UNLOCK(x)
#define ZCFIFO_LOCK_INIT(x)
#elif (ZC_FIFO_LOCK_MUTEX == 1)
#warning "fifo spin lock"
// typedef pthread_spinlock_t zcfifo_safe_lock_t;
#define zcfifo_safe_lock_t pthread_spinlock_t
#define ZCFIFO_LOCK(x) pthread_spin_lock(x)
#define ZCFIFO_UNLOCK(x) pthread_spin_unlock(x)
#define ZCFIFO_LOCK_INIT(x) pthread_spin_init(x, PTHREAD_PROCESS_PRIVATE)
#else
#warning "fifo mutex lock"
// 互斥锁测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/610ms=1600M/
// ThreadPutLock cnt[1000000]errcnt[1311692],ret[1024000000],cos[610]ms
// ThreadGetLock cnt[1000000]errcnt[555166],ret[1024000000],cos[610]ms
typedef pthread_mutex_t zcfifo_safe_lock_t;
#define ZCFIFO_LOCK(x) pthread_mutex_lock(x)
#define ZCFIFO_UNLOCK(x) pthread_mutex_unlock(x)
#define ZCFIFO_LOCK_INIT(x) pthread_mutex_init(x, NULL)
#endif

typedef struct _zcfifo_safe_ {
    unsigned char *buffer; /* the buffer holding the data */
    unsigned int size;     /* the size of the allocated buffer */
    unsigned int in;       /* data is added at offset (in % size) */
    unsigned int out;      /* data is extracted from off. (out % size) */
    zcfifo_safe_lock_t *lock;   /* protects concurrent modifications */
} zcfifo_safe_t;

extern zcfifo_safe_t *zcfifo_safe_init(unsigned char *buffer, unsigned int size, zcfifo_safe_lock_t *lock);
extern zcfifo_safe_t *zcfifo_safe_alloc(unsigned int size, zcfifo_safe_lock_t *lock);
extern void zcfifo_safe_free(zcfifo_safe_t *fifo);

unsigned int zcfifo_safe_put(zcfifo_safe_t *fifo, const unsigned char *buffer, unsigned int len);
unsigned int zcfifo_safe_get(zcfifo_safe_t *fifo, unsigned char *buffer, unsigned int len);
void zcfifo_safe_reset(zcfifo_safe_t *fifo);
unsigned int zcfifo_safe_len(zcfifo_safe_t *fifo);
bool zcfifo_safe_is_empty(zcfifo_safe_t *fifo);
bool zcfifo_safe_is_full(zcfifo_safe_t *fifo);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif
