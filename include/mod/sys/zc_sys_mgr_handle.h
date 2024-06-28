// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_SYS_MGR_HANDLE_H__
#define __ZC_SYS_MGR_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"

typedef enum {
    SYS_MGR_HDL_RESTART_E = 0,
    SYS_MGR_HDL_REGISTER_E,
    SYS_MGR_HDL_UNREGISTER_E,

    SYS_MGR_HDL_BUTT_E,
} sys_mgr_handle_e;

// TODO(zhoucc): handle msg define

// SysManager handle mod msg callback
typedef int (*SysMgrHandleMsgCb)(void *ptr, unsigned int type, void *indata, void *outdata);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_SYS_MGR_HANDLE_H__*/
