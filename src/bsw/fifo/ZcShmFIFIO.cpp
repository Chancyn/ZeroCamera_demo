// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// shm fifo

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ZcShmFIFO.hpp"
#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"

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

#define ZC_FILE_MODE (0644)    // (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define ZC_EVFIFO_SIZE (2048)  // evfifo

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
namespace zc {
CShmFIFO::CShmFIFO(unsigned int size, const char *name, unsigned char chn, bool bwrite) {
    if (!is_power_of_2(size)) {
        BUG_ON(size > 0x80000000);
        size = roundup_pow_of_two(size);
    }
    m_size = size;
    m_pfifo.out = 0;
    m_pfifo.shmid = 0;
    m_pfifo.fifo = nullptr;
    // /dev/shm/shmfifo
    snprintf(m_szfifoname, sizeof(m_szfifoname) - 1, "/tmp/shmfifo_%.8s", name);
    if (access(m_szfifoname, F_OK) != 0) {
        LOG_ERROR("[%s] filenotexist, touch", m_szfifoname);
        char cmd[256];
        snprintf(cmd, sizeof(cmd) - 1, "touch %s", m_szfifoname);
        system(cmd);
    }
    m_shmkey = _getkey(m_szfifoname, chn);
    m_bwrite = bwrite;

    // ev mkfifo
    snprintf(m_szevname, sizeof(m_szevname) - 1, "/tmp/evfifo_%.8s%d", name, chn);
}

int CShmFIFO::_getkey(const char *name, unsigned char chn) {
    key_t shmkey = ftok(name, chn);
    if (shmkey == -1) {
        LOG_ERROR("err ftok get key error ");
        shmkey = 0;
    }
    LOG_TRACE("_getkey [%s%d]->shmkey[%d]", name, chn, shmkey);
    return shmkey;
}

CShmFIFO::~CShmFIFO() {
    _shmfree();
}

bool CShmFIFO::ShmAlloc() {
    return _shmalloc(m_size, m_shmkey, m_bwrite);
}

bool CShmFIFO::_shmalloc(unsigned int size, int shmkey, bool bwrite) {
    int shmid = 0;
    int evfd = 0;
    void *p = nullptr;

    unsigned int shmsize = sizeof(zcshmbuf_t) + m_size;
    shmid = shmget(shmkey, 0, 0);
    if (shmid == -1) {
        // if (!bwrite) {
        //     LOG_ERROR("err read shmget error,shmkey[%d], errno[%d]", shmkey, errno);
        //     goto _err;
        // }
        shmid = shmget(shmkey, shmsize, IPC_CREAT | 0644);
        if (shmid == -1) {
            LOG_ERROR("err shmget ");
            goto _err;
        }
    }

    p = shmat(shmid, NULL, 0);
    if (p == reinterpret_cast<void *>(-1)) {
        p = nullptr;
        LOG_ERROR("err shmat .");
        goto _err_ctl;
    }

    if (mkfifo(m_szevname, ZC_FILE_MODE) != 0 && errno != EEXIST) {
        LOG_ERROR("ev mkfifo error[%s] errno[%d]", m_szevname, errno);
        ZC_ASSERT(0);
    }

    if (m_bwrite) {
        // open nonblcok
        // evfd = open(m_szevname, O_RDWR | O_NONBLOCK, 0);
        evfd = open(m_szevname, O_RDWR | O_NONBLOCK, 0);
    } else {
        // evfd = open(m_szevname, O_RDONLY | O_NONBLOCK, 0);
        evfd = open(m_szevname, O_RDONLY | O_NONBLOCK, 0);
    }

    if (evfd < 0) {
        LOG_ERROR("ev open error[%s] [%d]", m_szevname, errno);
        ZC_ASSERT(0);
        goto _err_open;
    }
    m_pfifo.out = 0;
    m_pfifo.shmid = shmid;
    m_pfifo.fifo = reinterpret_cast<zcshmbuf_t *>(p);
    m_pfifo.fifo->size = m_size;
    m_pfifo.fifo->in = m_pfifo.fifo->out = 0;
    m_pfifo.evfd = evfd;
    LOG_ERROR("shmalloc ok shmid[%d],shmkey[%d]", shmid, shmkey);
    return true;
_err_open:
    if (m_bwrite) {
        unlink(m_szevname);
    }
    shmdt(reinterpret_cast<zcshmbuf_t *>(p));
_err_ctl:
    shmctl(shmid, IPC_RMID, NULL);
_err:
    LOG_ERROR("shmalloc error shmid[%d],shmkey[%d]", shmid, shmkey);
    return false;
}

/**
 * zcfifo_free - frees the FIFO
 * @fifo: the fifo to be freed.
 */
void CShmFIFO::_shmfree() {
    if (m_pfifo.fifo) {
        LOG_TRACE("_shmfree shmdt shmid[%d]", m_pfifo.shmid);
        if (m_pfifo.evfd > 0) {
            close(m_pfifo.evfd);
            m_pfifo.evfd = 0;
            if (m_bwrite) {
                unlink(m_szevname);
            }
        }
        shmdt(m_pfifo.fifo);
        m_pfifo.fifo = nullptr;
        if (m_bwrite) {
            LOG_TRACE("_shmfree shmctl delete shmid[%d]", m_pfifo.shmid);
            shmctl(m_pfifo.shmid, IPC_RMID, NULL);
        }
        m_pfifo.shmid = 0;
    }

    return;
}
void CShmFIFO::ShmFree() {
    return _shmfree();
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
unsigned int CShmFIFO::_put(const unsigned char *buffer, unsigned int len) {
    ZC_ASSERT(m_pfifo.fifo != nullptr);
    unsigned int l;

    // len = min(len, m_pfifo.fifo->size - m_pfifo.fifo->in + m_pfifo.fifo->out);

    /*
     * Ensure that we sample the m_pfifo.fifo->out index -before- we
     * start putting bytes into the zcfifo.
     */
    // 加内存屏障，保证在开始放入数据之前，m_fifo.out取到正确的值（另一个CPU可能正在改写out值）
    smp_mb();

    /* first put the data starting from m_pfifo.fifo->in to buffer end */
    l = min(len, m_pfifo.fifo->size - (m_pfifo.fifo->in & (m_pfifo.fifo->size - 1)));
    memcpy(m_pfifo.fifo->buffer + (m_pfifo.fifo->in & (m_pfifo.fifo->size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(m_pfifo.fifo->buffer, buffer + l, len - l);

    /*
     * Ensure that we add the bytes to the zcfifo -before-
     * we update the m_pfifo.fifo->in index.
     */

    // 加写内存屏障，保证in 加之前，memcpy的字节已经全部写入buffer，
    // 如果不加内存屏障，可能数据还没写完，另一个CPU就来读数据，读到的缓冲区内的数据不完全，因为读数据是通过 in – out
    // 来判断的。
    smp_wmb();

    m_pfifo.fifo->in += len;

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
unsigned int CShmFIFO::_get(unsigned char *buffer, unsigned int len) {
    ZC_ASSERT(m_pfifo.fifo != nullptr);
    unsigned int l;

    len = min(len, m_pfifo.fifo->in - m_pfifo.fifo->out);

    /*
     * Ensure that we sample the m_pfifo.fifo->in index -before- we
     * start removing bytes from the zcfifo.
     */

    // 加读内存屏障，保证在开始取数据之前，m_fifo.in取到正确的值（另一个CPU可能正在改写in值）
    smp_rmb();

    /* first get the data from m_pfifo.fifo->out until the end of the buffer */
    l = min(len, m_pfifo.fifo->size - (m_pfifo.fifo->out & (m_pfifo.fifo->size - 1)));
    memcpy(buffer, m_pfifo.fifo->buffer + (m_pfifo.fifo->out & (m_pfifo.fifo->size - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, m_pfifo.fifo->buffer, len - l);

    /*
     * Ensure that we remove the bytes from the zcfifo -before-
     * we update the m_pfifo.fifo->out index.
     */

    // 加内存屏障，保证在修改out前，已经从buffer中取走了数据，
    // 如果不加屏障，可能先执行了增加out的操作，数据还没取完，令一个CPU可能已经往buffer写数据，将数据破坏，因为写数据是通过m_fifo.size
    // - (m_pfifo.fifo->in & (m_pfifo.fifo->size - 1))来判断的 。

    smp_mb();

    m_pfifo.fifo->out += len;

    return len;
}
unsigned int CShmFIFO::put(const unsigned char *buffer, unsigned int len) {
    unsigned int ret = 0;
    ret = _put(buffer, len);

    // write evfd
    if (ret && m_pfifo.evfd > 0 && write(m_pfifo.evfd, "w", 1) < 0) {
        // char buf[ZC_EVFIFO_SIZE];
        LOG_ERROR("warn write evfifo error, read clear");
        // read(m_pfifo.evfd, buf, sizeof(buf));
    }
    return ret;
}

unsigned int CShmFIFO::get(unsigned char *buffer, unsigned int len) {
    return _get(buffer, len);
}

// unsafe be careful use
void CShmFIFO::Reset() {
    ZC_ASSERT(m_pfifo.fifo != nullptr);
    m_pfifo.fifo->in = m_pfifo.fifo->out = 0;
    return;
}

unsigned int CShmFIFO::Len() {
    ZC_ASSERT(m_pfifo.fifo != nullptr);
    return m_pfifo.fifo->in - m_pfifo.fifo->out;
}

bool CShmFIFO::IsEmpty() {
    ZC_ASSERT(m_pfifo.fifo != nullptr);
    return (m_pfifo.fifo->in == m_pfifo.fifo->out);
}

bool CShmFIFO::IsFull() {
    ZC_ASSERT(m_pfifo.fifo != nullptr);
    return ((m_pfifo.fifo->in - m_pfifo.fifo->out) >= m_pfifo.fifo->size);
}

}  // namespace zc
