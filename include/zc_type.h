// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_TYPE_H__
#define __ZC_TYPE_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef unsigned int ZC_U32;
typedef int ZC_S32;
typedef unsigned short ZC_U16;
typedef short ZC_S16;
typedef unsigned char ZC_U8;
typedef signed char ZC_S8;
typedef long long ZC_S64;
typedef unsigned long long ZC_U64;
typedef float ZC_FLOAT;
typedef double ZC_DOUBLE;
typedef char ZC_CHAR;
typedef void *ZC_HANDLE;

#ifndef BOOL
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#endif

#define ZC_SUCCESS 0
#define ZC_FAILURE (-1)

#ifndef NULL
#define NULL (void *)0
#endif

#define _ALIGN(addr, T) ((unsigned)((char *)addr + sizeof(T) - 1) & ~(sizeof(T) - 1))
#define _ROUND(size, mask) (((size) + (mask)-1) & ~((mask)-1))
#define _SIZEOFTAB(x) (sizeof(x) / sizeof((x)[0]))

#ifdef __GNUC__
#define ZC_GCC_VERSION_AT_LEAST(x, y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#else
#define ZC_GCC_VERSION_AT_LEAST(x, y) 0
#endif

#ifndef zc_always_inline
#if ZC_GCC_VERSION_AT_LEAST(3, 1)
#define zc_always_inline __attribute__((always_inline)) inline
#else
#define zc_always_inline inline
#endif
#endif

#ifndef zc_extern_inline
#if defined(__ICL) && __ICL >= 1210 || defined(__GNUC_STDC_INLINE__)
#define zc_extern_inline extern inline
#else
#define zc_extern_inline inline
#endif
#endif

#if defined(__GNUC__)
#define zc_unused __attribute__((unused))
#else
#define zc_unused
#endif

#if defined(__GNUC__) && (__GNUC__ >= 4)
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#if defined(__GNUC__)
#define zc_packed __attribute__((packed))
#else
#define zc_packed
#endif

#if defined(__GNUC__)
#define ZC_CALL
#define ZC_API __attribute((visibility("default")))
#else
#define ZC_CALL __stdcall
#if defined(ZC_EXPORTS)
#define ZC_API __declspec(dllexport)
#else
#define ZC_API __declspec(dllimport)
#endif
#endif

#define ZC_SAFE_FREE(x) \
    do { \
        if ((x) != NULL) { \
            free(x); \
            x = NULL; \
        } \
    } while (0)


#define ZC_MAX_PATH 256

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
