// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_BASE_STREAM_H__
#define __ZC_BASE_STREAM_H__

#include <stdint.h>
#include "zc_frame.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef struct _zc_h26x_nalu {
    uint32_t type;          // zc_nalu_type_e
    uint32_t offset;        // start code offset
    uint32_t size;          // size not contain 0x00, 0x00, 0x00, 0x01
} zc_h26x_nalu_t;

typedef struct _zc_h26x_nalu_info {
    zc_h26x_nalu_t nalus[ZC_FRAME_NALU_MAXNUM];
} zc_h26x_nalu_info_t;
uint32_t zc_h264_nalu_val2type(uint8_t naluval);
uint32_t zc_h265_nalu_val2type(uint8_t naluval);
uint32_t zc_h26x_nalu_val2type(uint8_t naluval, int type);

uint32_t zc_h264_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *nalus);
uint32_t zc_h265_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *nalus);
uint32_t zc_h26x_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *info, int type);

const char* zc_get_livestreampath(zc_shmstream_e type);
// path: live.ch/pushs.ch/pull.ch
int zc_prase_livestreampath(const char *path,  zc_shmstream_e *type);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_BASE_STREAM_H__*/
