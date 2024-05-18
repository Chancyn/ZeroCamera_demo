// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo.move to userspace, lockfree fifo

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#include "zc_macros.h"
#include "zcfifo.h"


// userspace modify................

#ifndef smp_wmb
// #warning "zhoucc nodefine smp_wmb"
#ifdef __GNUC__
#define ZC_MBARRIER_FULL __sync_synchronize
#define smp_wmb ZC_MBARRIER_FULL
#define smp_mb ZC_MBARRIER_FULL
#define smp_rmb ZC_MBARRIER_FULL
#endif
#endif

#ifndef BUG_ON
#define BUG_ON(cond) ZC_ASSERT(!(cond))
#endif

#define min(x, y) ((x) < (y) ? (x) : (y))

// 一个数向上取整到最接近的2的幂
static inline unsigned int roundup_pow_of_two(unsigned int n) {
    if (n == 0) {
        // 如果输入是0，返回1，因为2的0次幂是1
        return 1;
    }
    // 找到n的最高位的1的位置，并将它右边的所有位都置为1
    // 这可以通过n减1，然后再取反加1来实现
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

#if 0
static inline unsigned int rounddown_pow_of_two(unsigned int n) {
    // 找到n中最高位的1，并把它以下的所有位都置为0
    // 这可以通过对n进行位与操作，操作数是n的二进制表示中最高位的1左移一位后减1得到的
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    // 由于上面的位或操作，n现在是一个所有低于最高位1的位都被置为1的数
    return (n >> 1) + 1;
}
#endif

static inline bool is_power_of_2(unsigned int n) {
    return (n != 0 && ((n & (n - 1)) == 0));
}
// userspace modify................

/**
 * zcfifo_init - allocates a new FIFO using a preallocated buffer
 * @buffer: the preallocated buffer to be used.
 * @size: the size of the internal buffer, this have to be a power of 2.
 *
 * Do NOT pass the zcfifo to zcfifo_free() after use! Simply free the
 * &zcfifo_t with kfree().
 */
zcfifo_t *zcfifo_init(unsigned char *buffer, unsigned int size) {
    zcfifo_t *fifo;

    /* size must be a power of 2 */
    BUG_ON(!is_power_of_2(size));

    fifo = malloc(sizeof(zcfifo_t));
    if (!fifo)
        return NULL;

    fifo->buffer = buffer;
    fifo->size = size;
    fifo->in = fifo->out = 0;

    return fifo;
}

/**
 * zcfifo_alloc - allocates a new FIFO and its internal buffer
 * @size: the size of the internal buffer to be allocated.
 *
 * The size will be rounded-up to a power of 2.
 */
zcfifo_t *zcfifo_alloc(unsigned int size) {
    unsigned char *buffer;
    zcfifo_t *ret;

    /*
     * round up to the next power of 2, since our 'let the indices
     * wrap' technique works only in this case.
     */
    if (!is_power_of_2(size)) {
        BUG_ON(size > 0x80000000);
        size = roundup_pow_of_two(size);
    }

    buffer = malloc(size);
    if (!buffer)
        return NULL;

    ret = zcfifo_init(buffer, size);

    if (!ret)
        free(buffer);

    return ret;
}

/**
 * zcfifo_free - frees the FIFO
 * @fifo: the fifo to be freed.
 */
void zcfifo_free(zcfifo_t *fifo) {
    free(fifo->buffer);
    free(fifo);
}

/**
 * zcfifo_put - puts some data into the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @buffer: the data to be added.
 * @len: the length of the data to be added.
 *
 * This function copies at most @len bytes from the @buffer into
 * the FIFO depending on the free space, and returns the number of
 * bytes copied.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
unsigned int zcfifo_put(zcfifo_t *fifo, const unsigned char *buffer, unsigned int len) {
    unsigned int l;

    len = min(len, fifo->size - fifo->in + fifo->out);

    /*
     * Ensure that we sample the fifo->out index -before- we
     * start putting bytes into the zcfifo.
     */
    // 加内存屏障，保证在开始放入数据之前，fifo->out取到正确的值（另一个CPU可能正在改写out值）
    smp_mb();

    /* first put the data starting from fifo->in to buffer end */
    l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(fifo->buffer, buffer + l, len - l);

    /*
     * Ensure that we add the bytes to the zcfifo -before-
     * we update the fifo->in index.
     */

    // 加写内存屏障，保证in 加之前，memcpy的字节已经全部写入buffer，
    // 如果不加内存屏障，可能数据还没写完，另一个CPU就来读数据，读到的缓冲区内的数据不完全，因为读数据是通过 in – out
    // 来判断的。
    smp_wmb();

    fifo->in += len;

    return len;
}

/**
 * zcfifo_get - gets some data from the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @buffer: where the data must be copied.
 * @len: the size of the destination buffer.
 *
 * This function copies at most @len bytes from the FIFO into the
 * @buffer and returns the number of copied bytes.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
unsigned int zcfifo_get(zcfifo_t *fifo, unsigned char *buffer, unsigned int len) {
    unsigned int l;

    len = min(len, fifo->in - fifo->out);

    /*
     * Ensure that we sample the fifo->in index -before- we
     * start removing bytes from the zcfifo.
     */

    // 加读内存屏障，保证在开始取数据之前，fifo->in取到正确的值（另一个CPU可能正在改写in值）
    smp_rmb();

    /* first get the data from fifo->out until the end of the buffer */
    l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, fifo->buffer, len - l);

    /*
     * Ensure that we remove the bytes from the zcfifo -before-
     * we update the fifo->out index.
     */

    // 加内存屏障，保证在修改out前，已经从buffer中取走了数据，
    // 如果不加屏障，可能先执行了增加out的操作，数据还没取完，令一个CPU可能已经往buffer写数据，将数据破坏，因为写数据是通过fifo->size
    // - (fifo->in & (fifo->size - 1))来判断的 。

    smp_mb();

    fifo->out += len;

    return len;
}

// nolcok version unsafe be careful use, stop read/write and reset it
void zcfifo_reset(zcfifo_t *fifo) {
    fifo->in = fifo->out = 0;
}

unsigned int zcfifo_len(zcfifo_t *fifo) {
    return fifo->in - fifo->out;
}

// bool zcfifo_is_empty(zcfifo_t *fifo) {
//     return (fifo->in == fifo->out);
// }

// // zcfifo_is_full - returns true if the fifo is full
// bool zcfifo_is_full(zcfifo_t *fifo) {
//     return ((fifo->in - fifo->out) >= fifo->size);
// }
