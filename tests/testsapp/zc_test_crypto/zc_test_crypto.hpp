// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// ./zc_tests 0 123456 / ./zc_tests 1 zc_tests
// type 0: for test md5sum string/buffer;
// type 1: for test md5sum file;
int zc_test_crypto_md5(int type, const char *pdata);
