// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_bsw_add.h"
#include "zc_plat_add.h"
#include "zc_test_add.h"

int main(int argc, char **argv) {
    printf("test into\n");
    int a = zc_test_add(10, 20);
    printf("test add a=%d\n", a);
    int b = zc_test_dec(a, 5);
    printf("test dec a=%d\n", b);
    printf("test exit\n");
    return 0;
}
