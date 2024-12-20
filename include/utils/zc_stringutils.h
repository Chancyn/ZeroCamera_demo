
// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_STRINGUTILS_H__
#define __ZC_STRINGUTILS_H__
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <stdio.h>

size_t zc_strlcpy(char *dst, const char *src, size_t size);
size_t zc_strlcat(char *dst, const char *src, size_t size);
size_t zc_strlcatf(char *dst, size_t size, const char *fmt, ...);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif
