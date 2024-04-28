// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_plat_add.h"

int zc_plat_add(int a, int b) {
#if ZC_DEBUG
    printf("plat debug a+b [%d]+[%d]", a, b);
#endif
    return a + b;
}

int zc_plat_dec(int a, int b) {
#if ZC_DEBUG
    printf("plat debug a-b [%d]-[%d]", a, b);
#endif
    return a - b;
}
