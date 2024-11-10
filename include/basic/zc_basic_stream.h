// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_BASE_STREAM_H__
#define __ZC_BASE_STREAM_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_frame.h"
#include <stdint.h>

typedef struct _zc_h26x_nalu {
    uint32_t type;    // zc_nalu_type_e
    uint32_t offset;  // start code offset
    uint32_t size;    // size not contain 0x00, 0x00, 0x00, 0x01
} zc_h26x_nalu_t;

typedef struct _zc_h26x_nalu_info {
    zc_h26x_nalu_t nalus[ZC_FRAME_NALU_MAXNUM];
} zc_h26x_nalu_info_t;

typedef struct _zc_mpeg4_aac {
    uint8_t profile;                   // 0-NULL, 1-AAC Main, 2-AAC LC, 2-AAC SSR, 3-AAC LTP
    uint8_t sampling_frequency_index;  // 0-96000, 1-88200, 2-64000, 3-48000, 4-44100, 5-32000, 6-24000, 7-22050,
                                       // 8-16000, 9-12000, 10-11025, 11-8000, 12-7350, 13/14-reserved, 15-frequency is
                                       // written explictly
    uint8_t channel_configuration;  // 0-AOT, 1-1channel,front-center, 2-2channels, front-left/right, 3-3channels: front
                                    // center/left/right, 4-4channels: front-center/left/right, back-center,
                                    // 5-5channels: front center/left/right, back-left/right, 6-6channels: front
                                    // center/left/right, back left/right LFE-channel, 7-8channels

    uint32_t sampling_frequency;              // codec frequency, valid only in decode
    uint32_t extension_frequency;             // play frequency(AAC-HE v1/v2 sbr/ps)
    uint8_t extension_audio_object_type;      // default 0, valid on sbr/ps flag
    uint8_t extension_channel_configuration;  // default: channel_configuration
    uint8_t channels;                         // valid only in decode
    int sbr;                                  // sbr flag, valid only in decode
    int ps;                                   // ps flag, valid only in decode
    uint8_t pce[64];
    int npce;  // pce bytes
} zc_mpeg4_aac_t;

uint32_t zc_h264_nalu_val2type(uint8_t naluval);
uint32_t zc_h265_nalu_val2type(uint8_t naluval);
uint32_t zc_h26x_nalu_val2type(uint8_t naluval, int type);

uint32_t zc_h264_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *nalus);
uint32_t zc_h265_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *nalus);
uint32_t zc_h26x_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *info, int type);

const char *zc_get_livestreampath(zc_shmstream_e type);
// path: live.ch/pushs.ch/pull.ch
int zc_prase_livestreampath(const char *path, zc_shmstream_e *type);
int zc_mpeg4_aac_adts_load(const uint8_t *data, size_t bytes, zc_mpeg4_aac_t *aac);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_BASE_STREAM_H__*/
