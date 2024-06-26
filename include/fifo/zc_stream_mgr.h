// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// shmstreammgr
#ifndef __ZC_STREAM_MGR_H__
#define __ZC_STREAM_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_frame.h"

// live channel num
#define ZC_STREAMMGR_LIVE_MAX_CHN 2

// pull channel num
#define ZC_STREAMMGR_PULLC_MAX_CHN 2

// push server channel num
#define ZC_STREAMMGR_PUSHS_MAX_CHN 2

// push client channel num
#define ZC_STREAMMGR_PUSHC_MAX_CHN 2

// decode channel num
#define ZC_STREAMMGR_DECODE_MAX_CHN 1

// stream with track num, video+audio, ZC_STREAM_BUTT
#define ZC_STREAMMGR_TRACK_MAX_NUM ZC_STREAM_BUTT  // TODO(zhoucc): just video+audio

#define ZC_SHMSTREAM_ALL (0xFFFF)

// video
typedef enum {
    // video
    ZC_TRACK_STATUS_ERROR = -1,
    ZC_TRACK_STATUS_IDE = 0,
    ZC_TRACK_STATUS_W,   // write open
    ZC_TRACK_STATUS_RW,  // write/read open

    ZC_TRACK_STATUS_BUTT,
} zc_track_status_e;

// video
typedef enum {
    // video
    ZC_STREAM_STATUS_ERROR = -1,
    ZC_STREAM_STATUS_INIT = 0,  // init
    ZC_STREAM_STATUS_IDE,       // ide
    ZC_STREAM_STATUS_RUNING,    // working

    ZC_STREAM_STATUS_BUTT,
} zc_stream_status_e;


typedef struct _zc_stream_mgr_cfg {
    unsigned char maxchn[ZC_SHMSTREAM_BUTT];

    unsigned char decode_maxchn;  // decode support max chn
} zc_stream_mgr_cfg_t;

// streammgr client info
typedef struct _zc_streamcli {
    ZC_U32 mod;
    ZC_S32 pid;             // pid_t
    unsigned int lasttime;  // recv last msg time, for keepalive

    char pname[64];  // process name
} zc_streamcli_t;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_STREAM_MGR_H__*/
