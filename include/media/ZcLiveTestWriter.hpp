// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#if ZC_LIVE_TEST
#include <stdint.h>

#include <memory>
#include <string>

// #include "time64.h"
#include "zc_frame.h"

typedef struct {
    unsigned int chn;
    unsigned int len;
    const uint8_t *ptr;
} test_raw_frame_t;

typedef enum {
    ZC_TEST_FILE_STREAM_RAW  = 0,      // H264/H265
    ZC_TEST_FILE_MP4,                  // mp4

    ZC_TEST_FILE_BUTT,
} zc_test_file_e;

typedef struct {
    unsigned int chn;
    unsigned int size;
    unsigned int file;    // zc_test_file_e
    unsigned int encode;  // ZC_FRAME_ENC_H265
    unsigned int fps;     // fps
    char fifopath[128];
    char filepath[128];
    char threadname[32];
} live_test_info_t;

namespace zc {
class ILiveTestWriter {
 public:
    ILiveTestWriter() {}
    virtual ~ILiveTestWriter() {}

 public:
    virtual int Init() = 0;
    virtual int UnInit() = 0;
};

}  // namespace zc
#endif
