#include <stdio.h>

#include "zc_basic_fun.h"
#include "zc_log.h"

void zc_debug_dump_binstream(const char *fun, int type, const uint8_t *data, uint32_t len) {
    #ifdef ZC_DUMP_BINSTREAM
    printf("[%s] type:%d, dump bin len:%u [", fun, type, len);
    for (unsigned int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("]\n");
    #endif
}

