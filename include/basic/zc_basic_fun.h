// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_BASE_FUN_H__
#define __ZC_BASE_FUN_H__

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "zc_frame.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

// debug dump stream
#ifdef ZC_DEBUG
#define ZC_DUMP_BINSTREAM 1
#endif

void zc_debug_dump_binstream(const char *fun, int type, const uint8_t *data, uint32_t len);
//////////////////////////////////////////////////////////////////////////
///
/// implement
///
//////////////////////////////////////////////////////////////////////////
/// milliseconds since the Epoch(1970-01-01 00:00:00 +0000 (UTC))
static inline uint64_t zc_system_time(void) {
#if defined(CLOCK_REALTIME)
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
#else
    // POSIX.1-2008 marks gettimeofday() as obsolete, recommending the use of clock_gettime(2) instead.
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

///@return milliseconds(relative time)
static inline uint64_t zc_system_clock(void) {
#if defined(CLOCK_MONOTONIC)
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
#else
    // POSIX.1-2008 marks gettimeofday() as obsolete, recommending the use of clock_gettime(2) instead.
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

static inline int zc_system_version(int *major, int *minor) {
    struct utsname ver;
    if (0 != uname(&ver))
        return errno;
    if (2 != sscanf(ver.release, "%8d.%8d", major, minor))
        return -1;
    return 0;
}

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_BASE_STREAM_H__*/
