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

#include "zc_binmsg.h"
#include "zc_log.h"

#if 1
static int zc_test_binmsg_sendreg(int modid, int modidto) {
    return 0;
}
#endif
static ZC_U16 g_seq = 0;

static char random_char() {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int key = rand() % (sizeof(charset) - 1);
    return charset[key];
}

static void generate_random_string(char *str, int length) {
    if (length < 1)
        return;

    for (int i = 0; i < length - 1; ++i) {
        str[i] = random_char();
    }
    str[length - 1] = '\0';
}
static int zc_test_binmsg_unpack(const ZC_U8 *buf, ZC_U16 len) {
    char msg_buf[sizeof(zc_binmsg_t) + ZC_BINMSG_MAXLEN] = {0};
    zc_binmsg_t *msg = (zc_binmsg_t *)msg_buf;
    zc_binmsg_unpackhdr(msg, buf, len);
    if (msg->size) {
        memcpy(msg->data, buf + sizeof(zc_binmsg_t), msg->size);
    }
    zc_binmsg_debug_printhdr(msg);
}

static int zc_test_binmsg_parse() {}

static int zc_test_binmsg_pack(bool border, bool bcrc32) {
    ZC_U16 cmd = rand() & 0xFFFF;

    char testdata[128] = {"testdata aaa bbxxjifajfmka\n"};
    char msg_buf[sizeof(zc_binmsg_t) + ZC_BINMSG_MAXLEN] = {0};
    ZC_U16 randlen = cmd % 64;
    ZC_U16 msglen = sizeof(zc_binmsg_t);
    ZC_U16 payloadlen = 0;
    zc_binmsg_t *msg = (zc_binmsg_t *)msg_buf;
    generate_random_string(testdata, randlen);
    LOG_INFO("test_binmsg into cmd:0x%04X, %u[%s], seq:%u\n", cmd, randlen, testdata, g_seq);
    zc_binmsg_packhdr(msg, border, bcrc32, 0, 1, ZC_BINMSG_TYPE_REQ_E, cmd, g_seq++);
    payloadlen = strlen(testdata) + 1;
    msglen = sizeof(zc_binmsg_t) + payloadlen;
    zc_binmsg_packdata(msg, (ZC_U8 *)testdata, payloadlen);
    zc_binmsg_debug_dump(msg);
    zc_test_binmsg_unpack((ZC_U8 *)msg, msglen);
    LOG_INFO("test_binmsg end cmd:0x%04X\n", cmd);
    return 0;
}

// start
int zc_test_binmsg_start(int count) {
    LOG_INFO("test_binmsg into,count:%d\n", count);
    srand((unsigned int)time(NULL));
    for (int i = 0; i < count; i++) {
        zc_test_binmsg_pack(false, false);
        zc_test_binmsg_pack(false, true);
        zc_test_binmsg_pack(true, false);
        zc_test_binmsg_pack(true, true);
    }
    LOG_INFO("test_binmsg end\n");
    return 0;
}

// stop
int zc_test_binmsg_stop() {
    return 0;
}
