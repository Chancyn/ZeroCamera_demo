// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "zc_basic_fun.h"
#include "zc_log.h"

void zc_debug_dump_binstream(const char *fun, int type, const uint8_t *data, uint32_t len) {
#ifdef ZC_DUMP_BINSTREAM
    printf("[%s] type:%d, dump bin len:%u [", fun, type, len);
    for (unsigned int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("]\n");
#endif
}

void _dumpTrackInfo(const char *user, zc_meida_track_t *info) {
    LOG_TRACE("[%s] ch:%u,trackno:%u,track:%u,encode:%u,en:%u,size:%u,fmaxlen:%u,status:%u,name:%s", user, info->chn,
              info->trackno, info->tracktype, info->encode, info->enable, info->fifosize, info->framemaxlen,
              info->status, info->name);
    return;
}

void _dumpStreamInfo(const char *user, zc_stream_info_t *info) {
    LOG_TRACE("[%s] type:%d,idx:%u,ch:%u,tracknum:%u,status:%u", user, info->shmstreamtype, info->idx, info->chn,
              info->tracknum, info->status);
    _dumpTrackInfo("vtrack", &info->tracks[ZC_STREAM_VIDEO]);
    _dumpTrackInfo("atrack", &info->tracks[ZC_STREAM_AUDIO]);
    _dumpTrackInfo("mtrack", &info->tracks[ZC_STREAM_META]);

    return;
}

char *GetTimeStr(char *dst, unsigned int len, time_t *now) {
    time_t tmp;
    if (now == NULL) {
        tmp = time(NULL);
        now = &tmp;
    }
    struct tm t;
    localtime_r(now, &t);
    // strftime(dst, len, "%Y-%m-%dT%H%M%S", &t);
    snprintf(dst, len, "%04d-%02d-%02dT%02d%02d%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
             t.tm_sec);
    return dst;
}

char* GenerateFileName(char *dst, unsigned int len) {
    time_t now = time(NULL);
    //unsigned int idx = rand_r((unsigned int *)&now) % 10000;
    srand(now);
    unsigned int idx = rand() % 10000;
    char tmstr[64];
    snprintf(dst, len, "%s_%04u", GetTimeStr(tmstr, sizeof(tmstr), &now), idx);
    return dst;
}

bool endsWith(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return false;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
        return false;
    }

    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

bool endsCaseWith(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return false;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
        return false;
    }

    return strncasecmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}