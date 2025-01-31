// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_MACROS_H__
#define __ZC_MACROS_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// 对齐到最接近的、不小于x的、alignment的倍数
#define ALIGN_UP(x, alignment) (((x) + ((alignment)-1)) & ~(((alignment)-1)))

// 对齐到最接近的、不大于x的、alignment的倍数
#define ALIGN_DOWN(x, alignment) ((x) & ~(((alignment)-1)))

#define ZCMKTAG(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define ZCMKBETAG(a, b, c, d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define ZCMAX(a, b) ((a) > (b) ? (a) : (b))
#define ZCMAX3(a, b, c) ZCMAX(ZCMAX(a, b), c)
#define ZCMIN(a, b) ((a) > (b) ? (b) : (a))
#define ZCMIN3(a, b, c) ZCMIN(ZCMIN(a, b), c)

#define ZC_USLEEP(Sec, Usec) \
    do { \
        struct timeval _SleepTime_; \
        _SleepTime_.tv_sec = Sec; \
        _SleepTime_.tv_usec = Usec; \
        select(1, NULL, NULL, NULL, &_SleepTime_); \
    } while (0)

#define ZC_MSLEEP(MSec) \
    do { \
        struct timeval _SleepTimeM_; \
        _SleepTimeM_.tv_sec = (MSec / 1000); \
        _SleepTimeM_.tv_usec = ((MSec % 1000) * 1000); \
        select(1, NULL, NULL, NULL, &_SleepTimeM_); \
    } while (0)

#ifdef ZC_DEBUG
#include <assert.h>
#define ZC_ASSERT(x) assert(x)
#else
#define ZC_ASSERT(x) \
    do { \
        LOG_ERROR("assert error"); \
    } while (0)
#endif

#if defined(_MSC_VER)
    #define ZC_UNUSED
#elif defined(__GNUC__)
    #define ZC_UNUSED __attribute__((unused))
#else
    #define ZC_UNUSED
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
