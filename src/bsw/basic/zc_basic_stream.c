// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zc_basic_stream.h"
#include "zc_h26x_sps_parse.h"
#include "zc_log.h"
#include "zc_type.h"

#define ZC_DEBUG_DUMP 0  // debug dump
#if ZC_DEBUG_DUMP
#include "zc_basic_fun.h"
#endif
static inline const uint8_t *search_start_code(const uint8_t *ptr, const uint8_t *end) {
    for (const uint8_t *p = ptr; p + 3 < end; p++) {
        if (0x00 == p[0] && 0x00 == p[1] && (0x01 == p[2] || (0x00 == p[2] && 0x01 == p[3])))
            return p;
    }
    return end;
}

uint32_t zc_h264_nalu_val2type(uint8_t naluval) {
    uint32_t typeval = naluval & 0x1F;
    uint32_t type = ZC_NALU_TYPE_UNKNOWN;
    if (H264_NAL_UNIT_TYPE_SPS == typeval) {
        type = ZC_NALU_TYPE_SPS;
    } else if (H264_NAL_UNIT_TYPE_PPS == typeval) {
        type = ZC_NALU_TYPE_PPS;
    } else if (H264_NAL_UNIT_TYPE_SEI == typeval) {
        type = ZC_NALU_TYPE_SEI;
    }

    return type;
}

uint32_t zc_h265_nalu_val2type(uint8_t naluval) {
    uint32_t typeval = (naluval & 0x7E) >> 1;
    uint32_t type = ZC_NALU_TYPE_UNKNOWN;
    if (H265_NAL_UNIT_VPS == typeval) {
        type = ZC_NALU_TYPE_VPS;
    } else if (H265_NAL_UNIT_SPS == typeval) {
        type = ZC_NALU_TYPE_SPS;
    } else if (H265_NAL_UNIT_PPS == typeval) {
        type = ZC_NALU_TYPE_PPS;
    } else if (H265_NAL_UNIT_PREFIX_SEI == typeval || H265_NAL_UNIT_SUFFIX_SEI == typeval) {
        type = ZC_NALU_TYPE_SEI;
    }

    return type;
}

uint32_t zc_h26x_nalu_val2type(uint8_t naluval, int type) {
    if (type == ZC_FRAME_ENC_H264)
        return zc_h264_nalu_val2type(naluval);
    else if (type == ZC_FRAME_ENC_H265)
        return zc_h265_nalu_val2type(naluval);

    return ZC_NALU_TYPE_UNKNOWN;
}

uint32_t zc_h264_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *nalus) {
    const uint8_t *end = data + dataSize;
    const uint8_t *start = search_start_code(data, end);
    if (start >= end) {
        return 0;
    }

    const uint8_t *p = start;
    uint32_t prefixlen = 0x01 == p[2] ? 3 : 4;
    uint32_t nalunum = 0;
#if ZC_DUMP_BINSTREAM  // dump
    zc_debug_dump_binstream(__FUNCTION__, ZC_FRAME_ENC_H264, p, dataSize, 64);
#endif
    while (p < end) {
        const unsigned char *pn = search_start_code(p + prefixlen, end);
        if (nalunum >= ZC_FRAME_NALU_MAXNUM) {
            LOG_WARN("nalu num:%d over%d", nalunum, ZC_FRAME_NALU_MAXNUM);
            break;
        }

        nalus->nalus[nalunum].type = zc_h264_nalu_val2type(p[prefixlen]);
        nalus->nalus[nalunum].size = pn - p - prefixlen;
        nalus->nalus[nalunum].offset = p - data;

        LOG_TRACE("h264 find idx:%d, type:%d, size:%u, offset:%u", nalunum, nalus->nalus[nalunum].type,
                  nalus->nalus[nalunum].size, nalus->nalus[nalunum].offset);
        nalunum++;
        p = pn;
    }

    return nalunum;
}

uint32_t zc_h265_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *nalus) {
    const uint8_t *end = data + dataSize;
    const uint8_t *start = search_start_code(data, end);
    if (start >= end) {
        return 0;
    }

    const uint8_t *p = start;
    uint32_t prefixlen = 0x01 == p[2] ? 3 : 4;
    uint32_t nalunum = 0;

#if ZC_DUMP_BINSTREAM  // dump
    zc_debug_dump_binstream(__FUNCTION__, ZC_FRAME_ENC_H265, p, dataSize, 64);
#endif

    while (p < end) {
        const unsigned char *pn = search_start_code(p + prefixlen, end);
        if (nalunum >= ZC_FRAME_NALU_MAXNUM) {
            LOG_WARN("nalu num:%d over%d", nalunum, ZC_FRAME_NALU_MAXNUM);
            break;
        }

        nalus->nalus[nalunum].type = zc_h265_nalu_val2type(p[prefixlen]);
        nalus->nalus[nalunum].size = pn - p - prefixlen;
        nalus->nalus[nalunum].offset = p - data;
        LOG_TRACE("h265 find idx:%d, type:%d, size:%u, offset:%u", nalunum, nalus->nalus[nalunum].type,
                  nalus->nalus[nalunum].size, nalus->nalus[nalunum].offset);
        nalunum++;
        p = pn;
    }

    return nalunum;
}

uint32_t zc_h26x_parse_nalu(const uint8_t *data, uint32_t dataSize, zc_h26x_nalu_info_t *info, int type) {
    if (data == NULL || dataSize == 0 || info == NULL) {
        return 0;
    }

    if (type == ZC_FRAME_ENC_H264)
        return zc_h264_parse_nalu(data, dataSize, info);
    else if (type == ZC_FRAME_ENC_H265)
        return zc_h265_parse_nalu(data, dataSize, info);

    return 0;
}

static const char *g_streamUrlTab[ZC_SHMSTREAM_BUTT] = {
    ZC_STREAM_LIVEURL_CHN_PREFIX,
    ZC_STREAM_PULLURL_CHN_PREFIX,
    ZC_STREAM_PUSHSURL_CHN_PREFIX,
    ZC_STREAM_PUSHCURL_CHN_PREFIX,
};

const char* zc_get_livestreampath(zc_shmstream_e type) {

    if (type >= ZC_SHMSTREAM_BUTT) {
        LOG_ERROR("type:%u, error", type);
        return NULL;
    }

    return g_streamUrlTab[type];
}

// path: live.ch/pushs.ch/pull.ch
int zc_prase_livestreampath(const char *path, zc_shmstream_e *type) {
    if (path == NULL || type == NULL) {
        return -1;
    }
    for (unsigned int i = 0; i < _SIZEOFTAB(g_streamUrlTab); i++) {
        int len = strlen(g_streamUrlTab[i]);
        if (0 == strncasecmp(path, g_streamUrlTab[i], len)) {
            *type = i;
            break;
        }
    }

    LOG_TRACE("%s, type:%d", path, *type);

    return 0;
}