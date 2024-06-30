// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_MEDIA_TRACK_H__
#define __ZC_MEDIA_TRACK_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"

typedef enum {
    ZC_MEDIA_TRACK_VIDEO = 0,  // a=control:video
    ZC_MEDIA_TRACK_AUDIO = 1,  // a=control:audio
    ZC_MEDIA_TRACK_META = 2,   // a=control:meta

    ZC_MEDIA_TRACK_BUTT,
} zc_media_track_e;

// video
typedef enum {
    // video
    ZC_MEDIA_CODE_H264 = 0,
    ZC_MEDIA_CODE_H265 = 1,

    // audio
    ZC_MEDIA_CODE_AAC = 10,

    // metadata
    ZC_MEDIA_CODE_METADATA = 20,

    ZC_MEDIA_CODE_BUTT,
} zc_media_code_e;
// shmstreamcfg

typedef struct _zc_media_track {
    unsigned char chn;        // chnno
    unsigned char trackno;    // tracktype zc_stream_e
    unsigned char tracktype;  // tracktype zc_stream_e
    unsigned char encode;     // encode zc_frame_enc_e
    unsigned int mediacode;   // encode zc_media_code_e for new different CMediaTrackH264
    unsigned int fifosize;    // shmfifosize
    unsigned char enable;     // enable/disable
    char name[32];         // shm path name
} zc_meida_track_t;

typedef struct _zc_stream_info {
    unsigned char shmstreamtype;  // live push pull zc_shmstream_type_e
    unsigned char idx;            // idx
    unsigned char chn;            // chn num
    unsigned char tracknum;       // num
    int status;                   // zc_stream_status_e, status: ide ;
    zc_meida_track_t tracks[ZC_MEDIA_TRACK_BUTT];
} zc_media_info_t;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_MEDIA_TRACK_H__*/
