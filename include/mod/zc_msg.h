// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_MSG_H__
#define __ZC_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"

#define ZC_URL_SIZE 128
#define ZC_DATETIME_STR_SIZE 128  // 2024-04-30 00:00:00

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
    ZC_U8 ver;        // version;
    ZC_U8 modid;      // modid,msg send moduleid zc_modid_e
    ZC_U8 msgtype;    // req/rep pub; zc_msg_type_e
    ZC_U8 cmd;        // get/set/ctrl; zc_msgcmd_type_e
    ZC_S8 chn;        // chn reserve data
    ZC_U8 rsv[3];     // reserve data
    ZC_U32 seq;       // seqno;
    ZC_U16 id;        // msgid;
    ZC_U16 sid;       // msgsubid;
    ZC_U32 ts;        // timestamp
    ZC_U32 ts1;       // timestamp1 for replay time
    ZC_S32 err;       // errorno
    ZC_U32 size;      // msgsize
    ZC_U8 data[0];    // msgdata
} zc_msg_t;

// typedef void(zc_msg_func_t)(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_MSG_H__*/