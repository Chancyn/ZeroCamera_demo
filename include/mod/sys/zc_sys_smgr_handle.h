// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_SYS_SMGR_HANDLE_H__
#define __ZC_SYS_SMGR_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"
#include "zc_stream_mgr.h"
// #include "zc_msg_sys.h"
#include "zc_frame.h"

// handle callback enum
typedef enum {
    SYS_SMGR_HDL_REGISTER_E = 0,  // register
    SYS_SMGR_HDL_UNREGISTER_E,    // unregister
    SYS_SMGR_HDL_GECOUNT_E,
    SYS_SMGR_HDL_GETALLINFO_E,
    SYS_SMGR_HDL_GETINFO_E,   // getsmgrinfo, in:zc_sys_smgr_getinfo_in_t, out:zc_sys_smgr_getinfo_out_t
    SYS_SMGR_HDL_SETINFO_E,  // setmgrinfo

    SYS_SMGR_HDL_BUTT_E,
} sys_smgr_handle_e;

// register ZC_MSID_SMGR_REGISTER_E
typedef struct {
    ZC_S32 pid;                   // pid
    ZC_U32 modid;                 // mod id
    ZC_S32 status;                // status
    ZC_CHAR pname[ZC_MAX_PNAME];  // process name
} zc_sys_smgr_reg_t;

// SYS_SMGR_HDL_GETINFO_E
typedef struct {
    ZC_U32 type;    // type:zc_shmstream_e
    ZC_U32 chn;
} zc_sys_smgr_getinfo_in_t;

typedef struct {
    zc_stream_info_t info;
} zc_sys_smgr_getinfo_out_t;

// SYS_SMGR_HDL_GECOUNT_E
typedef struct {
    ZC_U32 type;        // type
} zc_sys_smgr_getcount_in_t;

typedef struct {
    ZC_U32 count;
} zc_sys_smgr_getcount_out_t;

// SYS_SMGR_HDL_GETALLINFO_E
typedef struct {
    ZC_U32 type;        // type:zc_shmstream_e or ZC_SHMSTREAM_ALL
    ZC_U32 count;       // out buffer, zc_stream_info_t item count
} zc_sys_smgr_getallinfo_in_t;

typedef struct {
    zc_stream_info_t *pinfo;
} zc_sys_smgr_getallinfo_out_t;

// SYS_SMGR_HDL_GETINFO_E
typedef struct {
    ZC_U32 type;    // type:zc_shmstream_e
    ZC_U32 chn;
    zc_stream_info_t info;
} zc_sys_smgr_setinfo_in_t;

typedef struct {
    zc_stream_info_t info;       // return; set
} zc_sys_smgr_setinfo_out_t;
// streamMgr handle mod msg callback
typedef int (*SysStreamMgrHandleMsgCb)(void *ptr, unsigned int type, void *indata, void *outdata);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_SYS_SMGR_HANDLE_H__*/
