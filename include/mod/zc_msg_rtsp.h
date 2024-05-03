// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_MSG_RTSP_H__
#define __ZC_MSG_RTSP_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_msg.h"
#include "zc_type.h"

#define ZC_RTSP_URL_IPC "ipc:///tmp/rtsp_rep"  // req/rep url
#define ZC_RTSP_URL_PUB "ipc:///tmp/rtsp_pub"  // pub/sub url

// module rtsp, msg main id
typedef enum {
    ZC_MID_RTSP_MAN_E = 0,  // manager other module
    ZC_MID_RTSP_CFG_E,      // cfg
    ZC_MID_RTSP_CTRL_E,     // ctrl

    ZC_MID_RTSP_BUTT,  // end
} zc_mid_rtsp_e;

// msg sub id for manager
typedef enum {
    ZC_MSID_RTSP_MAN_VERSION_E = 1,  // version  submsg
    ZC_MSID_RTSP_MAN_RESTART_E,      // restart submsg
    ZC_MSID_RTSP_MAN_SHUTDOWN_E,     // shutdown submsg

    ZC_MSID_MAN_RTSP_BUTT,  // end
} zc_msid_rtsp_man_e;

// module system, msg sub id
typedef enum {
    ZC_MSID_RTSP_CFG_GET_E = 0,
    ZC_MSID_RTSP_CFG_SET_E = 0,

    ZC_MSID_RTSP_CFG_BUTT,  // end
} zc_msid_rtsp_cfg_e;

// module system, msg sub id
typedef enum {
    ZC_MSID_RTSP_REQIDR_E = 0,

    ZC_MSID_RTSP_CTRL_BUTT,  // end
} zc_msid_rtsp_ctrl_e;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_MSG_RTSP_H__*/
