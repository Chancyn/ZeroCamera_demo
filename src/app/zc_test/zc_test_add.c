// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_test_add.h"

int zc_test_add(int a, int b) {
#if ZC_DEBUG
    printf("debug a+b [%d]+[%d]", a, b);
#endif
    return a + b;
}

int zc_test_dec(int a, int b) {
#if ZC_DEBUG
    printf("debug a-b [%d]-[%d]", a, b);
#endif
    return a - b;
}
