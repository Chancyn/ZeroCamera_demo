#include <stdio.h>

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