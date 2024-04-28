// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_bsw_add.h"
#include "zc_log.h"

int zc_bsw_add(int a, int b) {
    LOG_DEBUG("bsw debug a+b [%d]+[%d]", a, b);

    return a + b;
}

int zc_bsw_dec(int a, int b) {
    LOG_WARN("bsw debug a-b [%d]-[%d]", a, b);

    return a - b;
}
