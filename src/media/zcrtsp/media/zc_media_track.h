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

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_MEDIA_TRACK_H__*/
