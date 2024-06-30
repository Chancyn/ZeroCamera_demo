// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// test crypto[md5sum/sha256/crc32/base64...] demo

#pragma once

/*
crypto:0 md5,1 crc32;
type:0 string, 1 file;
pdata:stringdata or filename

such as:
./zc_tests 0 0 123456       // for md5sum 123456
./zc_tests 0 1 filename     // for md5sum filename = shell cmd "md5sum filename"
./zc_tests 1 0 123456       // for crc 123456
./zc_tests 1 1 filename     // for crc filename = shell cmd "crc32 filename"
*/
int zc_test_crypto(int crypto, int type, const char *pdata, int datalen);
