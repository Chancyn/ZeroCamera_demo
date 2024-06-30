// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_MSG_CODEC_H__
#define __ZC_MSG_CODEC_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_msg.h"
#include "zc_type.h"

#
#define ZC_CODEC_MODNAME "modcodec"     // modname
// module codec, msg main id
typedef enum {
    ZC_MID_CODEC_MAN_E = 0,  // manager other module
    ZC_MID_CODEC_CFG_E,     // time
    ZC_MID_CODEC_E,     // base

    ZC_MID_CODEC_BUTT,  // end
} zc_mid_codec_e;

// msg sub id for manager
typedef enum {
    ZC_MSID_MAN_CODEC_VERSION_E = 0,  // version  submsg

    ZC_MSID_MAN_CODEC_BUTT,  // end
} zc_msid_codec_man_e;

// module system, msg sub id
typedef enum {
    ZC_MSID_CODEC_CFG_GET = 0,
    ZC_MSID_CODEC_CFG_SET = 0,

    ZC_MSID_CODEC_CFG_BUTT,  // end
} zc_msid_codec_cfg_e;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_MSG_CODEC_H__*/
