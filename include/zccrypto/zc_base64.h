// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from media-server, support base64_encode_url

#ifndef __ZC_BASE64_H__
#define __ZC_BASE64_H__

#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

// debug test
#if ZC_DEBUG
#define ZC_BASE64_DEBUG 1
#endif

/**
 * Calculate the output size in bytes needed to decode a base64 string
 * with length x to a data buffer.
 */
#define ZC_BASE64_DECODE_SIZE(x) ((x) * 3LL / 4 + 1)

/**
 * Calculate the output size needed to base64-encode x bytes to a
 * null-terminated string.
 */
#define ZC_BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)


/// base64 encode
/// @param[out] target output string buffer, target size = (source + 2)/3 * 4 + source / 57 (76/4*3)
/// @param[in] source input binary buffer
/// @param[in] bytes input buffer size in byte
/// @return target bytes
size_t zc_base64_encode(char* target, const void *source, size_t bytes);
/// same as base64_encode except "+/" -> "-_"
size_t zc_base64_encode_url(char* target, const void *source, size_t bytes);

/// base64 decode
/// @param[in] source input string buffer
/// @param[in] bytes input buffer size in byte
/// @param[out] target output binary buffer, target size = (source + 3)/4 * 3;
/// @return target bytes
size_t zc_base64_decode(void* target, const char *source, size_t bytes);

/// Base16(HEX) Encoding
/// @return target bytes
size_t zc_base16_encode(char* target, const void *source, size_t bytes);
size_t zc_base16_decode(void* target, const char *source, size_t bytes);

size_t zc_base32_encode(char* target, const void *source, size_t bytes);
size_t zc_base32_decode(void* target, const char *source, size_t bytes);

void zc_base64_debug_test(void);
#ifdef  __cplusplus
} // extern "C"
#endif

#endif /* !__ZC_BASE64_H__ */
