// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// reference monktan

// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_H26X_SPS_PARSE_H__
#define __ZC_H26X_SPS_PARSE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

// Table 7-1 NAL unit type codes
enum H264NalUnitType {
    H264_NAL_UNIT_TYPE_UNSPECIFIED = 0,                   // Unspecified
    H264_NAL_UNIT_TYPE_CODED_SLICE_NON_IDR = 1,           // Coded slice of a non-IDR picture
    H264_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A = 2,  // Coded slice data partition A
    H264_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B = 3,  // Coded slice data partition B
    H264_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C = 4,  // Coded slice data partition C
    H264_NAL_UNIT_TYPE_CODED_SLICE_IDR = 5,               // Coded slice of an IDR picture
    H264_NAL_UNIT_TYPE_SEI = 6,                           // Supplemental enhancement information (SEI)
    H264_NAL_UNIT_TYPE_SPS = 7,                           // Sequence parameter set
    H264_NAL_UNIT_TYPE_PPS = 8,                           // Picture parameter set
    H264_NAL_UNIT_TYPE_AUD = 9,                           // Access unit delimiter
    H264_NAL_UNIT_TYPE_END_OF_SEQUENCE = 10,              // End of sequence
    H264_NAL_UNIT_TYPE_END_OF_STREAM = 11,                // End of stream
    H264_NAL_UNIT_TYPE_FILLER = 12,                       // Filler data
    H264_NAL_UNIT_TYPE_SPS_EXT = 13,                      // Sequence parameter set extension
};

// H265 Table 7-1 NAL unit type codes and NAL unit type classes
enum H265NalUnitType {
    H265_NAL_UNIT_CODED_SLICE_TRAIL_N = 0,  // 0
    H265_NAL_UNIT_CODED_SLICE_TRAIL_R,      // 1

    H265_NAL_UNIT_CODED_SLICE_TSA_N,  // 2
    H265_NAL_UNIT_CODED_SLICE_TSA_R,  // 3

    H265_NAL_UNIT_CODED_SLICE_STSA_N,  // 4
    H265_NAL_UNIT_CODED_SLICE_STSA_R,  // 5

    H265_NAL_UNIT_CODED_SLICE_RADL_N,  // 6
    H265_NAL_UNIT_CODED_SLICE_RADL_R,  // 7

    H265_NAL_UNIT_CODED_SLICE_RASL_N,  // 8
    H265_NAL_UNIT_CODED_SLICE_RASL_R,  // 9

    H265_NAL_UNIT_RESERVED_VCL_N10,
    H265_NAL_UNIT_RESERVED_VCL_R11,
    H265_NAL_UNIT_RESERVED_VCL_N12,
    H265_NAL_UNIT_RESERVED_VCL_R13,
    H265_NAL_UNIT_RESERVED_VCL_N14,
    H265_NAL_UNIT_RESERVED_VCL_R15,

    H265_NAL_UNIT_CODED_SLICE_BLA_W_LP,    // 16
    H265_NAL_UNIT_CODED_SLICE_BLA_W_RADL,  // 17
    H265_NAL_UNIT_CODED_SLICE_BLA_N_LP,    // 18
    H265_NAL_UNIT_CODED_SLICE_IDR_W_RADL,  // 19
    H265_NAL_UNIT_CODED_SLICE_IDR_N_LP,    // 20
    H265_NAL_UNIT_CODED_SLICE_CRA,         // 21
    H265_NAL_UNIT_RESERVED_IRAP_VCL22,
    H265_NAL_UNIT_RESERVED_IRAP_VCL23,

    H265_NAL_UNIT_RESERVED_VCL24,
    H265_NAL_UNIT_RESERVED_VCL25,
    H265_NAL_UNIT_RESERVED_VCL26,
    H265_NAL_UNIT_RESERVED_VCL27,
    H265_NAL_UNIT_RESERVED_VCL28,
    H265_NAL_UNIT_RESERVED_VCL29,
    H265_NAL_UNIT_RESERVED_VCL30,
    H265_NAL_UNIT_RESERVED_VCL31,

    // non-VCL
    H265_NAL_UNIT_VPS,          // 32
    H265_NAL_UNIT_SPS,          // 33
    H265_NAL_UNIT_PPS,          // 34
    H265_NAL_UNIT_AUD,          // 35
    H265_NAL_UNIT_EOS,          // 36
    H265_NAL_UNIT_EOB,          // 37
    H265_NAL_UNIT_FILLER_DATA,  // 38
    H265_NAL_UNIT_PREFIX_SEI,   // 39
    H265_NAL_UNIT_SUFFIX_SEI,   // 40

    H265_NAL_UNIT_RESERVED_NVCL41,
    H265_NAL_UNIT_RESERVED_NVCL42,
    H265_NAL_UNIT_RESERVED_NVCL43,
    H265_NAL_UNIT_RESERVED_NVCL44,
    H265_NAL_UNIT_RESERVED_NVCL45,
    H265_NAL_UNIT_RESERVED_NVCL46,
    H265_NAL_UNIT_RESERVED_NVCL47,
    H265_NAL_UNIT_UNSPECIFIED_48,
    H265_NAL_UNIT_UNSPECIFIED_49,
    H265_NAL_UNIT_UNSPECIFIED_50,
    H265_NAL_UNIT_UNSPECIFIED_51,
    H265_NAL_UNIT_UNSPECIFIED_52,
    H265_NAL_UNIT_UNSPECIFIED_53,
    H265_NAL_UNIT_UNSPECIFIED_54,
    H265_NAL_UNIT_UNSPECIFIED_55,
    H265_NAL_UNIT_UNSPECIFIED_56,
    H265_NAL_UNIT_UNSPECIFIED_57,
    H265_NAL_UNIT_UNSPECIFIED_58,
    H265_NAL_UNIT_UNSPECIFIED_59,
    H265_NAL_UNIT_UNSPECIFIED_60,
    H265_NAL_UNIT_UNSPECIFIED_61,
    H265_NAL_UNIT_UNSPECIFIED_62,
    H265_NAL_UNIT_UNSPECIFIED_63,
    H265_NAL_UNIT_INVALID,
};

typedef struct _h26x_sps_info {
    uint32_t profile_idc;
    uint32_t level_idc;

    uint32_t width;
    uint32_t height;
    uint32_t fps;  // SPS中可能不包含FPS信息
} zc_h26x_sps_info_t;

/*
解析H264 SPS数据信息
@param data SPS数据内容，需要Nal类型为0x7数据的开始(比如：67 42 00 28 ab 40 22 01 e3 cb cd c0 80 80 a9 02)
@param dataSize SPS数据的长度
@param info SPS解析之后的信息数据结构体
@return success:0，fail:-1
*/
int32_t zc_h264_sps_parse(const uint8_t *data, uint32_t dataSize, zc_h26x_sps_info_t *info);

/*
解析H265 SPS数据信息
@param data SPS数据内容
@param dataSize SPS数据的长度
@param info SPS解析之后的信息数据结构体
@return success:0，fail:-1
*/
int32_t zc_h265_sps_parse(const uint8_t *data, uint32_t dataSize, zc_h26x_sps_info_t *info);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_H26X_SPS_PARSE_H__*/
