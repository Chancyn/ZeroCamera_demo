// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_RTSP_SMGR_HANDLE_H__
#define __ZC_RTSP_SMGR_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"

typedef enum {
    RTSP_SMGR_HDL_CHG_NOTIFY_E = 0,    // chg notify
    RTSP_SMGR_HDL_GETINFO_E,
    RTSP_SMGR_HDL_SETINFO_E,           // setmgrinfo

    RTSP_SMGR_HDL_BUTT_E,
} rtsp_smgr_handle_e;

// streamMgr handle mod msg callback
typedef int (*RtspStreamMgrHandleMsgCb)(void *ptr, unsigned int type, void *indata, void *outdata);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_RTSP_SMGR_HANDLE_H__*/
