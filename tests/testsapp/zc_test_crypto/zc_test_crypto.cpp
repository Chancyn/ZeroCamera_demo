// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "zc_base64.h"
#include "zc_crc32.h"
#include "zc_log.h"
#include "zc_md5.h"
#include "zc_type.h"

static int zc_test_crypto_md5_file(const char *file) {
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        LOG_ERROR("Error opening file:%s", file);
        return 1;
    }
    unsigned char md5Digest[MD5_SUM_LEN];
    ZC_MD5Context md5Context;

    ZC_MD5Init(&md5Context);
    unsigned char buffer[4096];
    size_t len;
    while ((len = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        ZC_MD5Update(&md5Context, buffer, len);
    }
    ZC_MD5Final(md5Digest, &md5Context);
    fclose(fp);
    LOG_INFO("test_crypto_md5 file[%s]->", file);
    for (int i = 0; i < MD5_SUM_LEN; ++i) {
        printf("%02x", md5Digest[i]);
    }
    printf("\n");
    return 0;
}

static int zc_test_crypto_md5_string(const unsigned char *data, size_t len) {
    unsigned char md5Digest[MD5_SUM_LEN];
    ZC_MD5Context md5Context;

    ZC_MD5Init(&md5Context);
    ZC_MD5Update(&md5Context, data, len);
    ZC_MD5Final(md5Digest, &md5Context);
    LOG_INFO("test_crypto_md5 string[%s]->", data);
    for (int i = 0; i < MD5_SUM_LEN; ++i) {
        printf("%02x", md5Digest[i]);
    }
    printf("\n");

    return 0;
}

// type 0: for test md5sum string/buffer;
// type 1: for test md5sum file;
static int zc_test_crypto_md5(int type, const char *pdata, int datalen) {
    LOG_INFO("test_crypto into,type:%d, pdata:%s\n", type, pdata);
    if (type == 1) {
        zc_test_crypto_md5_file(pdata);
    } else {
        zc_test_crypto_md5_string((const unsigned char *)pdata, datalen);
    }

    LOG_INFO("test_crypto end\n");
    return 0;
}

static int zc_test_crypto_crc32_file(const char *file) {
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        LOG_ERROR("Error opening file:%s", file);
        return 1;
    }

    ZC_U32 crc = 0;
    unsigned char buffer[4096];
    size_t len;
    while ((len = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        crc = crc32(crc, buffer, len);
    }

    fclose(fp);
    LOG_INFO("test_crypto_crc32 file[%s]->0x%08x", file, crc);
    return 0;
}

static int zc_test_crypto_crc32_string(const unsigned char *data, size_t len) {
    ZC_U32 crc = 0;
    crc = crc32(crc, data, len);
    LOG_INFO("test_crypto_crc32 string[%s]->0x%08x", data, crc);
    return 0;
}

static int zc_test_crypto_crc32(int type, const char *pdata, int datalen) {
    LOG_INFO("test_crypto into,type:%d, pdata:%s\n", type, pdata);
    if (type == 1) {
        zc_test_crypto_crc32_file(pdata);
    } else {
        zc_test_crypto_crc32_string((const unsigned char *)pdata, datalen);
    }

    LOG_INFO("test_crypto end\n");
    return 0;
}

static int zc_test_crypto_base64_encode(const char *data, size_t len) {
    size_t ret = 0;
    char dstbase64[ZC_BASE64_SIZE(len)];
    ret = zc_base64_encode(dstbase64, data, len);
    LOG_INFO("base64 encode [%zu][%s]->[%zu][%s]", len, data, ret, dstbase64);
    return 0;
}

static int zc_test_crypto_base64_encode_url(const char *data, size_t len) {
    size_t ret = 0;
    char dstbase64[ZC_BASE64_SIZE(len)];
    ret = zc_base64_encode_url(dstbase64, data, len);
    LOG_INFO("base64 encode url [%zu][%s]->[%zu][%s]", len, data, ret, dstbase64);
    return 0;
}

static int zc_test_crypto_base64_decode(const char *data, size_t len) {
    size_t ret = 0;
    char dst[ZC_BASE64_DECODE_SIZE(len)] = {0};
    ret = zc_base64_decode(dst, (const char *)data, len);
    LOG_INFO("base64 decode [%zu][%s]->[%zu][%s]", len, data, ret, dst);
    return 0;
}

// 0:base64-encode 1:base64-encode-url, 2 base64-decode
static int zc_test_crypto_base64(int type, const char *pdata, int datalen) {
    LOG_INFO("test_crypto base64 into,type:%d, pdata:%s\n", type, pdata);
    if (type == 0) {
        zc_test_crypto_base64_encode(pdata, datalen);
    } else if (type == 1) {
        zc_test_crypto_base64_encode_url(pdata, datalen);
    } else if (type == 2) {
        zc_test_crypto_base64_decode(pdata, datalen);
    } else if (type == 3) {
        zc_base64_debug_test();
    }

    return 0;
}

int zc_test_crypto(int crypto, int type, const char *pdata, int datalen) {
    if (pdata) {
        datalen = strlen(pdata);
    } else {
        datalen = 0;
    }
    LOG_INFO("test_crypto:%d, into,type:%d, len:%d, pdata:%s\n", crypto, type, datalen, pdata);

    if (crypto == 0) {
        zc_test_crypto_md5(type, pdata, datalen);
    } else if (crypto == 1) {
        zc_test_crypto_crc32(type, pdata, datalen);
    } else if (crypto == 2) {
        zc_test_crypto_base64(type, pdata, datalen);
    } else {
        LOG_INFO("unsupport crypto:%d end\n", crypto);
    }

    LOG_INFO("test_crypto end\n");
    return 0;
}
