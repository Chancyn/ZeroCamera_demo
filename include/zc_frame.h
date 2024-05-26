// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_FRAME_H__
#define __ZC_FRAME_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "zc_type.h"

#define ZC_STREAM_VIDEO_SHM_PATH "video"
#define ZC_STREAM_AUDIO_SHM_PATH "audio"
#define ZC_STREAM_MAIN_VIDEO_SIZE (8 * 1024 * 1024)
#define ZC_STREAM_SUB_VIDEO_SIZE (8 * 1024 * 1024)
#define ZC_STREAM_AUDIO_SIZE (1 * 1024 * 1024)
#define ZC_STREAM_MAXFRAME_SIZE (1 * 1024 * 1024)
#define ZC_STREAM_TEST_CHN (1)  // test chn
#define ZC_STREAM_VIDEO_MAX_CHN (2)  // video stream max chn
#define ZC_FRAME_NALU_MAX_SIZE (6)  // max nalu size

// video
typedef enum {
    // video
    ZC_STREAM_VIDEO = 0,
    ZC_STREAM_AUDIO,
    ZC_STREAM_META,

    ZC_STREAM_BUTT,
} zc_stream_e;

typedef enum {
    ZC_FRAME_ENC_H264 = 0,
    ZC_FRAME_ENC_H265,
    ZC_FRAME_ENC_AAC,

    ZC_FRAME_ENC_BUTT,
} zc_frame_enc_e;

typedef enum {
    ZC_FRAME_P = 0,
    ZC_FRAME_IDR,
    ZC_FRAME_I,

    ZC_FRAME_BUTT,
} zc_frame_e;

// video frame info
typedef struct {
    ZC_U8 encode;                         // zc_frame_enc_e
    ZC_U8 frame;                          // zc_frame_e
    ZC_U8 res[2];                         // res;
    ZC_U16 width;                         // picture width;
    ZC_U16 height;                        // picture height;
    ZC_U32 nalu[ZC_FRAME_NALU_MAX_SIZE];  // nalu info for create sdp
} zc_frame_video_t;

// audio frame info
typedef struct {
    ZC_U8 encode;       // zc_frame_enc_e
    ZC_U8 chn;          // chn number
    ZC_U8 bitwidth;     // bit width
    ZC_U8 res[1];       // res;
    ZC_U16 samplerate;  // sample rate(k)
    ZC_U16 res1;        // res;
    ZC_U32 res2[6];     // res;
} zc_frame_audio_t;

// meta frame info TODO
typedef struct {
    ZC_U8 encode;    //
    ZC_U8 res[3];    // res;
    ZC_U32 res1[7];  // res;
} zc_frame_meta_t;

// stream
typedef struct _zc_frame_ {
    ZC_U8 type;     // zc_stream_e
    ZC_U8 keyflag;  // idr flag;
    ZC_U16 seq;     // seqno;
    ZC_U32 size;    // data size;
    ZC_U32 utc;     // localtime ms;
    ZC_U32 pts;     // timestamp ms;
    ZC_U32 res[4];  // rsv
    union {
        zc_frame_video_t video;
        zc_frame_audio_t audio;
        zc_frame_meta_t meta;
    };
    ZC_U8 data[0];  // frame raw data
} zc_frame_t;

typedef unsigned int (*stream_puting_cb)(void *u, void *stream);
typedef unsigned int (*stream_geting_cb)(void *u, void *stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
