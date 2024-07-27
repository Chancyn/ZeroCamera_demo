// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_FRAME_H__
#define __ZC_FRAME_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "zc_media_track.h"
#include "zc_type.h"

// rtsp-server live shmfifo
#define ZC_STREAM_VIDEO_SHM_PATH "v_live"
#define ZC_STREAM_AUDIO_SHM_PATH "a_live"
#define ZC_STREAM_META_SHM_PATH "m_live"

// push-server -recv shmfifo
#define ZC_STREAM_VIDEOPUSH_SHM_PATH "v_pushs"  // video push fifo / or client recv fifo
#define ZC_STREAM_AUDIOPUSH_SHM_PATH "a_pushs"  // audio push fifo / or client recv fifo
#define ZC_STREAM_METAPUSH_SHM_PATH "m_pushs"

// rtsp-cli pull recv shmfifo
#define ZC_STREAM_VIDEOPULL_SHM_PATH "v_pull"  // video push fifo / or client recv fifo
#define ZC_STREAM_AUDIOPULL_SHM_PATH "a_pull"  // audio push fifo / or client recv fifo
#define ZC_STREAM_METAPULL_SHM_PATH "m_pull"

/*
sever url pathname
rtsp live url = rtsp://host[:port]/live/live.ch[chnnum]
http-flv live url = http://host[:port]/live/live.ch[chnnum].flv
ws-flv live url = ws://host[:port]/live/live.ch[chnnum].ws.flv
http-mp4 live url = http://host[:port]/live/live.ch[chnnum].mp4
ws-mp4 live url = ws://host[:port]/live/live.ch[chnnum].ws.mp4
for example:
rtsp://192.168.1.166:8554/live/live.ch0
rtsp://192.168.1.166/live/pull.ch1
http://192.168.1.166/live/live.ch0.flv
http://192.168.1.166/live/pull.ch0.mp4
*/

#define ZC_STREAM_LIVEURL_CHN_PREFIX "live"    // livestream prefix url
#define ZC_STREAM_PUSHSURL_CHN_PREFIX "pushs"  // rtsppush prefix url, forwarding stream prefix url
#define ZC_STREAM_PULLURL_CHN_PREFIX "pull"    // rtpcli pull forwarding prefix url
#define ZC_STREAM_PUSHCURL_CHN_PREFIX "pushc"  // rtpcli pull forwarding prefix url

// stream fifo max size
#define ZC_STREAM_MAIN_VIDEO_SIZE (8 * 1024 * 1024)
#define ZC_STREAM_SUB_VIDEO_SIZE (8 * 1024 * 1024)
#define ZC_STREAM_AUDIO_SIZE (1 * 1024 * 1024)
#define ZC_STREAM_META_SIZE (1 * 1024 * 1024)

// frame max size
#define ZC_STREAM_MAXFRAME_SIZE (1 * 1024 * 1024)  // video frame
#define ZC_STREAM_MAXFRAME_SIZE_A (2 * 1024)       // audio frame
#define ZC_STREAM_MAXFRAME_SIZE_M (2 * 1024)       // meta frame

#define ZC_STREAM_TEST_CHN (1)             // test chns
#define ZC_STREAM_VIDEO_MAX_CHN (2)        // video stream max chn
#define ZC_FRAME_NALU_MAXNUM (6)           // max nalu size
#define ZC_FRAME_NALU_BUFF_MAX_SIZE (256)  // sdp buffer max size

#define ZC_FRAME_VIDEO_MAGIC (0x5A435645)  // "ZCVE"
#define ZC_FRAME_AUDIO_MAGIC (0x5A434155)  // "ZCAU"
#define ZC_FRAME_META_MAGIC (0x5A434D54)   // "ZCME"

#define ZC_AUDIO_CHN (2)  // channel
#define ZC_AUDIO_FREQUENCE (48000)     // frequence


// shmstream type
typedef enum {
    ZC_SHMSTREAM_LIVE = 0,  // live shmstream,url /live/live.ch0
    ZC_SHMSTREAM_PULLC,     // pull cli shmstream, server forwarding url /live/pull.ch0
    ZC_SHMSTREAM_PUSHS,     // push server shmstream pushurl:/push/push.ch0, server url:/live/push.ch0
    ZC_SHMSTREAM_PUSHC,     // push client shmstream

    ZC_SHMSTREAM_BUTT,
} zc_shmstream_e;

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

    ZC_FRAME_ENC_META_BIN,  // user data binary data
    ZC_FRAME_ENC_BUTT,
} zc_frame_enc_e;

typedef enum {
    ZC_FRAME_P = 0,
    ZC_FRAME_IDR,
    ZC_FRAME_I,
    ZC_FRAME_I_BLA,   // hevc BLA
    ZC_FRAME_I_CRA,   // hevc CRA

    ZC_FRAME_BUTT,
} zc_frame_e;

// zc h26x nalu type
typedef enum {
    ZC_NALU_TYPE_VPS = 0,
    ZC_NALU_TYPE_SPS,
    ZC_NALU_TYPE_PPS,
    ZC_NALU_TYPE_SEI,

    ZC_NALU_TYPE_UNKNOWN = 0xFF,  // unknown, no need
} zc_nalu_type_e;

// zc h26x nalu type
typedef enum {
    ZC_AUDIO_SAMPLE_BIT_16 = 2,  // 16bit

    ZC_AUDIO_SAMPLE_BUTT,  //
} zc_audio_sample_bits_e;

typedef struct {
    ZC_U32 type;                              // zc_nalu_type_e
    ZC_U32 size;                              // size not contain 0x00, 0x00, 0x00, 0x01
    ZC_U8 data[ZC_FRAME_NALU_BUFF_MAX_SIZE];  // nalu info for create sdp
} zc_nalu_t;

// video nalu for package sdp
typedef struct {
    ZC_U16 width;   // picture width;
    ZC_U16 height;  // picture height;
    ZC_U16 fps;
    ZC_U16 nalunum;
    zc_nalu_t nalu[ZC_FRAME_NALU_MAXNUM];  // res;
} zc_video_naluinfo_t;

// video nalu for package sdp
typedef struct {
    zc_nalu_t nalu[1];  // res;
} zc_audio_naluinfo_t;

// video frame info
typedef struct {
    ZC_U8 encode;                       // zc_frame_enc_e
    ZC_U8 frame;                        // zc_frame_e
    ZC_U8 nalunum;                      // nalu number;
    ZC_U8 res[1];                       // res;
    ZC_U16 width;                       // picture width;
    ZC_U16 height;                      // picture height;
    ZC_U32 nalu[ZC_FRAME_NALU_MAXNUM];  // size not contain 0x00, 0x00, 0x00, 0x01 nalu info for create sdp
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
    ZC_U32 magic;   // magic hdr; 0x5A,0x43,0x5A,0x43
    ZC_U32 size;    // data size;
    ZC_U8 type;     // zc_stream_e
    ZC_U8 keyflag;  // idr flag;
    ZC_U8 res0[2];
    ZC_U32 utc;     // localtime ms;
    ZC_U32 pts;     // timestamp ms;
    ZC_U32 seq;     // seqno;           // zhoucc
    ZC_U32 res[3];  // rsv
    union {
        zc_frame_video_t video;
        zc_frame_audio_t audio;
        zc_frame_meta_t meta;
    };
    ZC_U8 data[0];  // frame raw data
} zc_frame_t;

// shmstream userbuf,
typedef struct _zc_frame_userinfo_ {
    ZC_U32 setflag;  // set flag
    ZC_U8 type;      // zc_stream_e
    ZC_U8 encode;    // zc_frame_enc_e
    ZC_U8 rsv[2];    // rsv
    ZC_U32 rsv1[4];  // rsv
    union {
        zc_video_naluinfo_t vinfo;
        zc_audio_naluinfo_t ainfo;
    };
} zc_frame_userinfo_t;

// stream mgr
// video nalu for package sdp
typedef struct {
    ZC_U16 width;   // picture width;
    ZC_U16 height;  // picture height;
    ZC_U16 fps;
    ZC_U16 rsv;      // rsv
    ZC_U32 rsv1[4];  // rsv
} zc_video_trackinfo_t;

// audio info channel,samplerate...
typedef struct {
    ZC_U8 channels;      ///< number of audio channels
    ZC_U8 sample_bits;   ///< bits per sample 0,ZC_AUDIO_SAMPLE_BIT_16
    ZC_U16 sample_rate;  ///< samples per second(frequency)
} zc_audio_trackinfo_t;

typedef struct _zc_media_track {
    unsigned char chn;         // chnno
    unsigned char trackno;     // tracktype zc_stream_e
    unsigned char tracktype;   // tracktype zc_stream_e
    unsigned char encode;      // encode zc_frame_enc_e
    unsigned int mediacode;    // encode zc_media_code_e for new different CMediaTrackH264
    unsigned int fifosize;     // shmfifosize
    unsigned int framemaxlen;  // frame maxlen
    unsigned char enable;      // enable/disable
    union {
        zc_video_trackinfo_t vtinfo;
        zc_audio_trackinfo_t atinfo;
    };
    int status;
    char name[32];  // shm path name
} zc_meida_track_t;

typedef struct _zc_stream_info {
    unsigned char shmstreamtype;  // live push pull zc_shmstream_e
    unsigned char idx;            // idx
    unsigned char chn;            // chn num
    unsigned char tracknum;       // num
    unsigned char fps;            // fps
    int status;                   // zc_stream_status_e, status: ide ;
    zc_meida_track_t tracks[ZC_MEDIA_TRACK_BUTT];
} zc_stream_info_t;

typedef unsigned int (*stream_puting_cb)(void *u, void *stream);
typedef unsigned int (*stream_geting_cb)(void *u, void *stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
