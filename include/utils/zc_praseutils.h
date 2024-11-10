// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_PRASEUTILS_H__
#define __ZC_PRASEUTILS_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <stdio.h>

int zc_prase_key_value(const char *src, const char *key, char *value, int vlen);
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif