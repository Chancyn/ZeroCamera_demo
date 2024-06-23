// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "zc_basic_stream.h"
#include "zc_log.h"

static inline const uint8_t *search_start_code(const uint8_t *ptr, const uint8_t *end) {
    for (const uint8_t *p = ptr; p + 3 < end; p++) {
        if (0x00 == p[0] && 0x00 == p[1] && (0x01 == p[2] || (0x00 == p[2] && 0x01 == p[3])))
            return p;
    }
    return end;
}

static uint32_t zc_h264_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *nalus) {
    const uint8_t *end = data + dataSize;
    const uint8_t *start = search_start_code(data, end);
    if (start >= end) {
        return 0;
    }

    const uint8_t *p = start;
    uint32_t prefixlen =  0x01 == p[2] ? 3 : 4;
    uint32_t nalunum = 0;
    #if 1 // dump
    for (int i = 0; i < 64; i++) {
        printf("%02x ", p[i]);
    }
    printf("\n");
    #endif
    while (p < end) {
        const unsigned char *pn = search_start_code(p + prefixlen, end);
        if (nalunum >= ZC_FRAME_NALU_MAX_SIZE) {
            LOG_WARN("nalu num:%d over%d", nalunum, ZC_FRAME_NALU_MAX_SIZE);
            break;
        }
        nalus->nalus[nalunum].type = p[prefixlen] & 0x1F;
        nalus->nalus[nalunum].size = pn - p - prefixlen;
        nalus->nalus[nalunum].offset = p - data;
        LOG_TRACE("find idx:%d, type:%d, size:%u, offset:%u", nalunum, nalus->nalus[nalunum].type, nalus->nalus[nalunum].size,
        nalus->nalus[nalunum].offset);
        nalunum++;
        p = pn;
    }

    return nalunum;
}

uint32_t zc_h26x_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *info) {
    if (data == NULL || dataSize == 0 || info == NULL) {
        return 0;
    }

    return zc_h264_parse_nalu(data, dataSize, info);
}
