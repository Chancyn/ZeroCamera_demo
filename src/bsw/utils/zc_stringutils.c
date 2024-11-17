// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zc_stringutils.h"

size_t zc_strlcpy(char *dst, const char *src, size_t size) {
    size_t len = 0;
    while (++len < size && *src)
        *dst++ = *src++;
    if (len <= size)
        *dst = 0;
    return len + strlen(src) - 1;
}

size_t zc_strlcat(char *dst, const char *src, size_t size) {
    size_t len = strlen(dst);
    if (size <= len + 1)
        return len + strlen(src);
    return len + zc_strlcpy(dst + len, src, size - len);
}

size_t zc_strlcatf(char *dst, size_t size, const char *fmt, ...) {
    size_t len = strlen(dst);
    va_list vl;

    va_start(vl, fmt);
    len += vsnprintf(dst + len, size > len ? size - len : 0, fmt, vl);
    va_end(vl);

    return len;
}
