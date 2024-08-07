// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_MSG_SYS_H__
#define __ZC_MSG_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_frame.h"
#include "zc_msg.h"
#include "zc_type.h"

#define ZC_SYS_MODNAME "modsys"  // modname

// module system, msg main id
typedef enum {
    ZC_MID_SYS_MAN_E = 0,  // manager other module
    ZC_MID_SYS_SMGR_E,     // streamMgr ctrl

    ZC_MID_SYS_TIME_E,  // time
    ZC_MID_SYS_BASE_E,  // base
    ZC_MID_SYS_USER_E,  // user
    ZC_MID_SYS_UPG_E,   // upg

    ZC_MID_SYS_BUTT,  // end
} zc_mid_sys_e;

// msg sub id for manager ZC_MID_SYS_MAN_E
typedef enum {
    ZC_MSID_SYS_MAN_REGISTER_E = 0,  // register submsg
    ZC_MSID_SYS_MAN_VERSION_E,       // version  submsg
    ZC_MSID_SYS_MAN_RESTART_E,       // restart submsg
    ZC_MSID_SYS_MAN_SHUTDOWN_E,      // shutdown submsg
    ZC_MSID_SYS_MAN_KEEPALIVE_E,     // keepalive

    ZC_MSID_SYS_MAN_BUTT,  // end
} zc_msid_sys_man_e;

// register cmd
typedef enum {
    ZC_SYS_UNREGISTER_E = 0,  // unregister submsg
    ZC_SYS_REGISTER_E,        // register submsg

    ZC_SYS_REGISTER_BUTT,  // end
} zc_sys_register_e;

// msg sub id for streamMgr ZC_MID_SYS_STREAM_E
typedef enum {
    ZC_MSID_SMGR_REGISTER_E = 0,  // register , streamMgrCli send register to streamMgr
    ZC_MSID_SMGR_UNREGISTER_E,    // unregister
    ZC_MSID_SMGR_GET_E,           // set info
    ZC_MSID_SMGR_SET_E,           // set info
    ZC_MSID_SMGR_GETALL_E,        // set info

    ZC_MSID_SMGR_BUTT,  // end
} zc_msid_sys_streammgr_e;

// module system, msg sub id ZC_MID_SYS_TIME_E
typedef enum {
    ZC_MSID_SYS_TIME_GET_E = 0,
    ZC_MSID_SYS_TIME_SET_E,

    ZC_MSID_SYS_TIME_BUTT,  // end
} zc_msid_sys_time_e;

// module system, msg sub id ZC_MID_SYS_BASE_E
typedef enum {
    ZC_MSID_SYS_BASE_GET_E = 0,
    ZC_MSID_SYS_BASE_SET_E,

    ZC_MSID_SYS_BASE_BUTT,  // end
} zc_msid_sys_base_e;

// module system, msg sub id ZC_MID_SYS_USER_E
typedef enum {
    ZC_MSID_SYS_USER_GET_E = 0,
    ZC_MSID_SYS_USER_SET_E,

    ZC_MSID_SYS_USER_BUTT,  // end
} zc_msid_sys_user_e;

// module system, msg sub id ZC_MID_SYS_UPG_E
typedef enum {
    ZC_MSID_SYS_UPG_START_E = 0,
    ZC_MSID_SYS_UPG_STOP_E,

    ZC_MSID_SYS_UPG_BUTT,  // end
} zc_msid_sys_upg_e;

// register ZC_MSID_SYS_MAN_REGISTER_E
typedef struct {
    ZC_CHAR date[ZC_DATETIME_STR_SIZE];  // build time
    ZC_CHAR url[ZC_URL_SIZE];            // url
    ZC_S32 regcmd;                       // 0 unregister, 1 register zc_sys_register_e
    ZC_S32 regstatus;                    // registerstatus
    // ZC_U32 modid;                     // mod id
    // ZC_S32 pid;                       // pid
    ZC_U32 ver;                   // mod version
    ZC_CHAR pname[ZC_MAX_PNAME];  // build time

} zc_mod_reg_t;

// keepalive ZC_MSID_SYS_MAN_KEEPALIVE_E
typedef struct {
    ZC_U32 seqno;                        // mod version
    ZC_S32 status;                       // mod status
    ZC_U8 rsv[4];                        // mod rsv
    ZC_CHAR date[ZC_DATETIME_STR_SIZE];  // mod rsv
} zc_mod_keepalive_t;

// rep keepalive ZC_MSID_SYS_MAN_KEEPALIVE_E
typedef struct {
    ZC_U32 seqno;                        // mod version
    ZC_S32 status;                       // mod status
    ZC_U8 rsv[4];                        // mod rsv
    ZC_CHAR date[ZC_DATETIME_STR_SIZE];  // mod rsv
} zc_mod_keepalive_rep_t;

// register ZC_MSID_SMGR_REGISTER_E
typedef struct {
    ZC_S32 pid;                   // pid
    ZC_U32 modid;                 // mod id
    ZC_S32 status;                // status
    ZC_U32 rsv1[16];              // rsv
    ZC_CHAR pname[ZC_MAX_PNAME];  // process name
} zc_mod_smgr_reg_t;

// register ZC_MSID_SMGR_UNREGISTER_E
typedef struct {
    ZC_S32 pid;                   // pid
    ZC_U32 modid;                 // mod ida
    ZC_S32 status;                // status
    ZC_U32 rsv1[16];              // rsv
    ZC_CHAR pname[ZC_MAX_PNAME];  // process name
} zc_mod_smgr_unreg_t;

// ZC_MSID_SMGR_SET_E
typedef struct {
    ZC_U32 type;     // live/pullc/pushs/pushc type
    ZC_U32 chn;      // channel
    ZC_U32 rsv1[6];  // rsv
    zc_stream_info_t info;
} zc_mod_smgr_set_t;

typedef struct {
    zc_stream_info_t info;
} zc_mod_smgr_set_rep_t;

// ZC_MSID_SMGR_GET_E
typedef struct {
    ZC_U32 type;     // live/pullc/pushs/pushc type
    ZC_U32 chn;      // channel
    ZC_U32 rsv1[6];  // rsv
} zc_mod_smgr_get_t;

typedef struct {
    zc_stream_info_t info;
} zc_mod_smgr_get_rep_t;

// ZC_MSID_SMGR_GETALL_E
typedef struct {
    ZC_U32 type;     // live/pullc/pushs/pushc type
    ZC_U32 rsv1[7];  // rsv
} zc_mod_smgr_getall_t;

typedef struct {
    ZC_U32 itemcount;  //  0 get one streaminfo,get all streaminfo
    ZC_U32 itemsize;   // live/pullc/pushs/pushc type
    ZC_U32 rsv1[8];    // rsv
} zc_mod_smgr_getall_rep_t;

/***********************publish & subcribe msg************************************/
// pub/sub msg main id
typedef enum {
    ZC_PUBMID_SYS_MAN = 0,  // sys recv mananger register msg, publishing to all module
    ZC_PUBMID_SYS_EV,

    ZC_PUBMID_SYS_BUTT,
} zc_pubmid_sys_e;

typedef enum {
    ZC_PUBMSID_SYS_MAN_REG = 0,        // sys recv register msg, publishing to other module
    ZC_PUBMSID_SYS_MAN_STREAM_UPDATE,  // sys stream mananger ecv streaminfo set, publishing to other module
    ZC_PUBMSID_SYS_MAN_CFG,            // sys recv cfg set, publishing to other module

    ZC_PUBMSID_SYS_MAN_BUTT,
} zc_pubmsid_sys_man_e;

typedef enum {
    ZC_PUBMSID_SYS_EV_AI = 0,  // sys recv event msg, publishing to other module

    ZC_PUBMSID_SYS_EV_BUTT,
} zc_pubmsid_sys_ev_e;

// ZC_PUBMSID_SYS_MAN_REG
typedef struct {
    ZC_CHAR url[ZC_URL_SIZE];            // url
    ZC_S32 regstatus;                    // registerstatus
    ZC_U32 modid;                        // mod id
    ZC_S32 pid;                          // pid
    ZC_U32 ver;                          // mod version
    ZC_U64 regtime;                      // reg time
    ZC_U64 lasttime;                     // last msg time
    ZC_CHAR pname[ZC_MAX_PNAME];         // build time
} zc_mod_pub_reg_t;

// ZC_PUBMSID_SYS_MAN_STREAM_UPDATE
typedef struct {
    ZC_U32 type;     // live/pullc/pushs/pushc type
    ZC_U32 chn;      // channel
    ZC_U32 rsv1[6];  // rsv
} zc_mod_pub_streamupdate_t;

// int zc_msg_func_proc(zc_msg_t *rep, int iqsize, zc_msg_t *rep, int *opsize);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_MSG_SYS_H__*/
