// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_RTSP_MGR_HANDLE_H__
#define __ZC_RTSP_MGR_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"

// TODO(zhoucc): handle cmd type
typedef enum {
    RTSP_MGR_HDL_RESTART_E = 0,

    RTSP_MGR_HDL_BUTT_E,
} rtsp_mgr_handle_e;

typedef enum {
    RTSP_MGR_HDL_SUB_STREAMUPDATE_E = 0,

    RTSP_MGR_HDL_SUB_BUTT_E,
} rtsp_mgr_handle_sub_e;

// RtspManager handle mod msg callback
typedef int (*RtspMgrHandleMsgCb)(void *ptr, unsigned int type, void *indata, void *outdata);
// RtspManager handle mod subscribe msg callback rtsp_mgr_handle_sub_e
typedef int (*RtspMgrHandleSubMsgCb)(void *ptr, unsigned int type, void *indata);

// TODO(zhoucc): handle msg define

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_RTSP_MGR_HANDLE_H__*/
