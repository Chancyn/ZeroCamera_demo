// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo.move to userspace, freelock fifo,
// no locking version just use 1 producer 1 consumer

#ifndef __ZCFIFO_H__
#define __ZCFIFO_H__
// copy from linux kernel(2.6.32) zcfifo.move to userspace, freelock fifo

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <stdbool.h>

typedef struct _zcfifo_ {
    unsigned char *buffer; /* the buffer holding the data */
    unsigned int size;     /* the size of the allocated buffer */
    unsigned int in;       /* data is added at offset (in % size) */
    unsigned int out;      /* data is extracted from off. (out % size) */
} zcfifo_t;

zcfifo_t *zcfifo_init(unsigned char *buffer, unsigned int size);
zcfifo_t *zcfifo_alloc(unsigned int size);
void zcfifo_free(zcfifo_t *fifo);

// zcfifo_put - puts some data into the FIFO, no locking version
unsigned int zcfifo_put(zcfifo_t *fifo, const unsigned char *buffer, unsigned int len);

// zcfifo_put - puts some data into the FIFO, no locking version
unsigned int zcfifo_get(zcfifo_t *fifo, unsigned char *buffer, unsigned int len);

//  @fifo: the fifo to be emptied. not safe
// void zcfifo_reset(zcfifo_t *fifo);

// zcfifo_len - returns the number of bytes available in the FIFO no locking version

unsigned int zcfifo_len(zcfifo_t *fifo);

#define zcfifo_is_empty(fifo) ({ fifo->in == fifo->out; })
// zcfifo_len - returns true if the fifo is empty no locking version
// bool zcfifo_is_empty(zcfifo_t *fifo);

#define zcfifo_is_full(fifo) ({ (fifo->in - fifo->out) >= fifo->size; })
// zcfifo_is_full - returns true if the fifo is full no locking version
// bool zcfifo_is_full(zcfifo_t *fifo);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif
