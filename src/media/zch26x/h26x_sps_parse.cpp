//
// Created by monktan on 2020/10/21.
//
#include "zc_log.h"

#include "h265_stream.h"
#include "zc_h26x_sps_parse.h"

int32_t zc_h264_sps_parse(const uint8_t *data, uint32_t dataSize, zc_h26x_sps_info_t *info) {
    if (data == nullptr || dataSize == 0)
        return -1;

    int32_t result = -1;
    h264_stream_t *h = h264_new();
    result = read_nal_unit(h, (uint8_t *)data, dataSize);
    if (result == -1) {
        LOG_ERROR("h264_read_nal_unit error.");
        h264_free(h);
        return -1;
    }

    if (h->nal->nal_unit_type != NAL_UNIT_TYPE_SPS) {
        LOG_ERROR("input not sps data.");
        h264_free(h);
        return -1;
    }

    if (info) {
        info->width = h->info->width;
        info->height = h->info->height;
        info->profile_idc = h->info->profile_idc;
        info->level_idc = h->info->level_idc;
        info->fps = h->info->max_framerate;
    }

    h264_free(h);

    return 0;
}

int32_t zc_h265_sps_parse(const uint8_t *data, uint32_t dataSize, zc_h26x_sps_info_t *info) {
    if (data == nullptr || dataSize == 0)
        return -1;

    int32_t result = -1;
    h265_stream_t *h = h265_new();
    result = h265_read_nal_unit(h, (uint8_t *)data, dataSize);
    if (result == -1) {
        LOG_ERROR("h265_read_nal_unit error.");
        h265_free(h);
        return -1;
    }

    if (h->nal->nal_unit_type != NAL_UNIT_SPS) {
        LOG_ERROR("input not sps data.");
        h265_free(h);
        return -1;
    }

    if (info) {
        info->width = h->info->width;
        info->height = h->info->height;
        info->profile_idc = h->info->profile_idc;
        info->level_idc = h->info->level_idc;
        info->fps = h->info->max_framerate;
    }
    h265_free(h);

    return 0;
}
