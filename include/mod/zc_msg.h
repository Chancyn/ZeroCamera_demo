// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_MSG_H__
#define __ZC_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"

#define ZC_MSG_TRANSPORT_IPC 0
#define ZC_MSG_TRANSPORT_TCP 1
#define ZC_MSG_TRANSPORT_UDP 0

#if ZC_MSG_TRANSPORT_IPC
#define ZC_SYS_URL_IPC "ipc:///tmp/sys_rep"      // req/rep url
#define ZC_SYS_URL_PUB "ipc:///tmp/sys_pub"      // pub/sub url
#define ZC_CODEC_URL_IPC "ipc:///tmp/codec_rep"  // req/rep url
#define ZC_CODEC_URL_PUB "ipc:///tmp/codec_pub"  // pub/sub url
#define ZC_RTSP_URL_IPC "ipc:///tmp/rtsp_rep"    // req/rep url
#define ZC_RTSP_URL_PUB "ipc:///tmp/rtsp_pub"    // pub/sub url
#elif ZC_MSG_TRANSPORT_TCP
#define ZC_SYS_URL_IPC "tcp://127.0.0.1:8880"    // req/rep url
#define ZC_SYS_URL_PUB "tcp://127.0.0.1:8881"    // pub/sub url
#define ZC_CODEC_URL_IPC "tcp://127.0.0.1:8882"  // req/rep url
#define ZC_CODEC_URL_PUB "tcp://127.0.0.1:8883"  // pub/sub url
#define ZC_RTSP_URL_IPC "tcp://127.0.0.1:8884"   // req/rep url
#define ZC_RTSP_URL_PUB "tcp://127.0.0.1:8885"   // pub/sub url
#elif ZC_MSG_TRANSPORT_UDP
#define ZC_SYS_URL_IPC "udp://127.0.0.1:8880"    // req/rep url
#define ZC_SYS_URL_PUB "udp://127.0.0.1:8881"    // pub/sub url
#define ZC_CODEC_URL_IPC "udp://127.0.0.1:8882"  // req/rep url
#define ZC_CODEC_URL_PUB "udp://127.0.0.1:8883"  // pub/sub url
#define ZC_RTSP_URL_IPC "udp://127.0.0.1:8884"   // req/rep url
#define ZC_RTSP_URL_PUB "udp://127.0.0.1:8885"   // pub/sub url
#endif

#define ZC_MSG_VERSION (1)  // version 1
#define ZC_MODNAME_SIZE 32
#define ZC_URL_SIZE 128
#define ZC_DATETIME_STR_SIZE 32   // 2024-04-30 00:00:00
#define ZC_MSG_ERRDETAIL_SIZE 64  // err detail

typedef enum {
    ZC_MSG_ERR_RIGHT_E = -7,       // msglen right error
    ZC_MSG_ERR_PARAM_E = -6,       // msglen param error
    ZC_MSG_ERR_CMDID_E = -5,       // error cmdid
    ZC_MSG_ERR_LEN_E = -4,         // msglen error
    ZC_MSG_ERR_LICENSE_E = -3,     // license error
    ZC_MSG_ERR_UNREGISTER_E = -2,  // unregister error
    ZC_MSG_ERR_E = -1,             // err
    ZC_MSG_SUCCESS_E = 0,          // handle success < 0: not handle;
    ZC_MSG_WARN_PARAM_E,           // Handle warning PARAM error

    ZC_MSG_ERR_BUTT,  // end
} zc_msg_errcode_e;

// modid
typedef enum {
    ZC_MODID_SYS_E = 0,  // mod system mod
    ZC_MODID_CODEC_E,    // codec mod
    ZC_MODID_RTSP_E,     // rtsp mod
    ZC_MODID_WEB_E,      // web mod
    ZC_MODID_UI_E,       // gui mod

    ZC_MODID_BUTT,  // end
} zc_modid_e;

typedef enum {
    // req/rep
    ZC_MSG_TYPE_REQ_E = 0,  // request
    ZC_MSG_TYPE_REP_E,      // replay

    // pub-sub
    ZC_MSG_TYPE_PUB_E,  // publishing/subscribe

    // reserve for bus
    ZC_MSG_TYPE_BUS_E,

    ZC_MSG_TYPE_BUTT,  // end
} zc_msg_type_e;

typedef enum {
    ZC_MSGCMD_GET_E = 0,  // get cfg
    ZC_MSGCMD_SET_E,      // set cfg

    ZC_MSGCMD_STATUS_E,  // status
    ZC_MSGCMD_CTRL_E,    // ctrl

    ZC_MSGCMD_BUT_E,  // end
} zc_msgcmd_type_e;

// msg def
typedef struct {
    ZC_S32 pid;     // pid
    ZC_U32 modid;   // modid,msg send moduleid zc_modid_e
    ZC_U8 modidto;  // modid,msg send moduleid zc_modid_e
    ZC_U8 ver;      // version;
    ZC_U8 msgtype;  // req/rep pub; zc_msg_type_e
    ZC_U8 cmd;      // get/set/ctrl; zc_msgcmd_type_e
    ZC_S8 chn;      // chn reserve data
    ZC_U8 rsv[3];   // reserve data
    ZC_U32 seq;     // seqno;
    ZC_U16 id;      // msgid;
    ZC_U16 sid;     // msgsubid;
    ZC_U32 ts;      // timestamp
    ZC_U32 ts1;     // timestamp1 for replay time
    ZC_S32 err;     // errorno errcode
    ZC_U32 size;    // msgsize
    ZC_U8 data[0];  // msgdata
} zc_msg_t;

// typedef void(zc_msg_func_t)(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);

/***********************all moudle common msg ************************************************* */
// streammgr, msg sub id
typedef enum {
    ZC_MSID_SMGRCLI_CHGNOTIFY_E = 0,  // info change notify

    ZC_MSID_SMGRCLI_BUTT,  // end
} zc_msid_smgrcli_e;

//  ZC_MSID_SMGRCLI_CHGNOTIFY_E
typedef struct {
    ZC_U8 modid;  // moudleid
    ZC_U8 rsv[3];
    ZC_U8 rsv1[4];
} zc_mod_smgrcli_chgnotify_t;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_MSG_H__*/
