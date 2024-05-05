// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo.move to userspace, freelock fifo

#ifndef __ZCFIFO_H__
#define __ZCFIFO_H__
// copy from linux kernel(2.6.32) zcfifo.move to userspace, freelock fifo

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <pthread.h>  // for spinlock
#include <stdbool.h>

// 定义锁类型 0无锁/1自旋锁/2互斥锁/
#define ZC_FIFO_LOCK_MUTEX 1

#if (ZC_FIFO_LOCK_MUTEX == 0)
#warning "fifo no lock"
typedef int zcfifo_lock_t;
#define ZCFIFO_LOCK(x)
#define ZCFIFO_UNLOCK(x)
#define ZCFIFO_LOCK_INIT(x)
#elif (ZC_FIFO_LOCK_MUTEX == 1)
#warning "fifo spin lock"
// typedef pthread_spinlock_t zcfifo_lock_t;
#define zcfifo_lock_t pthread_spinlock_t
#define ZCFIFO_LOCK(x) pthread_spin_lock(x)
#define ZCFIFO_UNLOCK(x) pthread_spin_unlock(x)
#define ZCFIFO_LOCK_INIT(x) pthread_spin_init(x, PTHREAD_PROCESS_PRIVATE)
#else
#warning "fifo mutex lock"
// 互斥锁测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/610ms=1600M/
// ThreadPutLock cnt[1000000]errcnt[1311692],ret[1024000000],cos[610]ms
// ThreadGetLock cnt[1000000]errcnt[555166],ret[1024000000],cos[610]ms
typedef pthread_mutex_t zcfifo_lock_t;
#define ZCFIFO_LOCK(x) pthread_mutex_lock(x)
#define ZCFIFO_UNLOCK(x) pthread_mutex_unlock(x)
#define ZCFIFO_LOCK_INIT(x) pthread_mutex_init(x, NULL)
#endif

typedef struct _zcfifo_ {
    unsigned char *buffer; /* the buffer holding the data */
    unsigned int size;     /* the size of the allocated buffer */
    unsigned int in;       /* data is added at offset (in % size) */
    unsigned int out;      /* data is extracted from off. (out % size) */
    zcfifo_lock_t *lock;   /* protects concurrent modifications */
} zcfifo_t;

extern zcfifo_t *zcfifo_init(unsigned char *buffer, unsigned int size, zcfifo_lock_t *lock);
extern zcfifo_t *zcfifo_alloc(unsigned int size, zcfifo_lock_t *lock);
extern void zcfifo_free(zcfifo_t *fifo);
extern unsigned int __zcfifo_put(zcfifo_t *fifo, const unsigned char *buffer, unsigned int len);
extern unsigned int __zcfifo_get(zcfifo_t *fifo, unsigned char *buffer, unsigned int len);

/**
 * __zcfifo_reset - removes the entire FIFO contents, no locking version
 * @fifo: the fifo to be emptied.
 */
static inline void __zcfifo_reset(zcfifo_t *fifo) {
    fifo->in = fifo->out = 0;
}

/**
 * zcfifo_reset - removes the entire FIFO contents
 * @fifo: the fifo to be emptied.
 */
static inline void zcfifo_reset(zcfifo_t *fifo) {
    ZCFIFO_LOCK(fifo->lock);
    __zcfifo_reset(fifo);
    ZCFIFO_UNLOCK(fifo->lock);
}

/**
 * zcfifo_put - puts some data into the FIFO
 * @fifo: the fifo to be used.
 * @buffer: the data to be added.
 * @len: the length of the data to be added.
 *
 * This function copies at most @len bytes from the @buffer into
 * the FIFO depending on the free space, and returns the number of
 * bytes copied.
 */
static inline unsigned int zcfifo_put(zcfifo_t *fifo, const unsigned char *buffer, unsigned int len) {
    unsigned int ret;

    ZCFIFO_LOCK(fifo->lock);
    ret = __zcfifo_put(fifo, buffer, len);
    ZCFIFO_UNLOCK(fifo->lock);

    return ret;
}

/**
 * zcfifo_get - gets some data from the FIFO
 * @fifo: the fifo to be used.
 * @buffer: where the data must be copied.
 * @len: the size of the destination buffer.
 *
 * This function copies at most @len bytes from the FIFO into the
 * @buffer and returns the number of copied bytes.
 */
static inline unsigned int zcfifo_get(zcfifo_t *fifo, unsigned char *buffer, unsigned int len) {
    unsigned int ret;

    ZCFIFO_LOCK(fifo->lock);
    ret = __zcfifo_get(fifo, buffer, len);
#if 0  // zhoucc nolock donot set 0
    /*
     * optimization: if the FIFO is empty, set the indices to 0
     * so we don't wrap the next time
     */

    if (fifo->in == fifo->out)
        fifo->in = fifo->out = 0;
#endif

    ZCFIFO_UNLOCK(fifo->lock);

    return ret;
}

/**
 * __zcfifo_len - returns the number of bytes available in the FIFO, no locking version
 * @fifo: the fifo to be used.
 */
static inline unsigned int __zcfifo_len(zcfifo_t *fifo) {
    return fifo->in - fifo->out;
}

/**
 * zcfifo_len - returns the number of bytes available in the FIFO
 * @fifo: the fifo to be used.
 */
static inline unsigned int zcfifo_len(zcfifo_t *fifo) {
    unsigned int ret;

    ZCFIFO_LOCK(fifo->lock);
    ret = __zcfifo_len(fifo);
    ZCFIFO_UNLOCK(fifo->lock);

    return ret;
}
#define __zcfifo_is_empty(fifo) ({ fifo->in == fifo->out; })

static inline bool zcfifo_is_empty(zcfifo_t *fifo) {
    bool ret;

    ZCFIFO_LOCK(fifo->lock);
    ret = __zcfifo_is_empty(fifo);
    ZCFIFO_UNLOCK(fifo->lock);

    return ret;
}

/**
 * __zcfifo_is_full - returns true if the fifo is full
 * @fifo: address of the fifo to be used
 */
#define __zcfifo_is_full(fifo) ({ (fifo->in - fifo->out) >= fifo->size; })

static inline bool zcfifo_is_full(zcfifo_t *fifo) {
    bool ret;

    ZCFIFO_LOCK(fifo->lock);
    ret = __zcfifo_is_full(fifo);
    ZCFIFO_UNLOCK(fifo->lock);

    return ret;
}

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif
