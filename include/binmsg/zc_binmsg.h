// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// implement binary msg define,and pack/unpack interface

#ifndef __ZC_BINMSG_H__
#define __ZC_BINMSG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "zc_type.h"

#define ZC_BINMSG_MAXLEN (65535)
#define ZC_BINMSG_VERSION (1)  // version 1
// magic ZM
#define ZC_BINMSG_MAGIC0 (0x5A)  // Z
#define ZC_BINMSG_MAGIC1 (0x4D)  // M

#if ZC_DEBUG
#define ZC_BINMSG_DEBUG 1 // 0, debug
#endif

typedef enum {
    // req/rep
    ZC_BINMSG_TYPE_REQ_E = 0,  // request
    ZC_BINMSG_TYPE_RSP_E,      // response

    ZC_BINMSG_TYPE_BUTT,  // end
} zc_binmsg_type_e;

// __attribute__((packed)), pack 1 byte
#pragma pack(push)
#pragma pack(1)
// msg def
typedef struct {
    ZC_U8 magic[2];  // magic 0;
    struct _flags {
        ZC_U8 order;  // order:0 host, 1 net;
        ZC_U8 crc : 1;    // crc32 check:
        ZC_U8 rsv : 7;
    }flags;

    ZC_U32 crc32;   // crc32 checksum from[version->data]
    ZC_U8 version;  // version 1;
    ZC_U8 id;       // id src,
    ZC_U8 idto;     // id dst,
    ZC_U8 msgtype;  // 0 req; 1 rsp zc_binmsg_type_e

    ZC_U8 res0[2];  // res
    ZC_U16 cmd;     // cmd id

    ZC_U16 seq;
    ZC_U16 size;    // msgsize

    ZC_U8 res[16];  // reserve
    ZC_U8 data[0];  // msgdata
} zc_binmsg_t;

#pragma pack(pop)
typedef int (*ReadCb)(ZC_U8 *buf, int len);

void zc_binmsg_packhdr(zc_binmsg_t *msg, BOOL netorder, BOOL crc32, ZC_U8 id, ZC_U8 idto, ZC_U8 type, ZC_U16 cmd,
                       ZC_U16 seq);
void zc_binmsg_packdata(zc_binmsg_t *msg, const ZC_U8 *data, ZC_U16 len);
int zc_binmsg_unpackhdr(zc_binmsg_t *msg, const ZC_U8 *buf, ZC_U16 len);

// parse msg;
// return msgpos, >=0 parse ok, <0 parse error
int zc_binmsg_parse(zc_binmsg_t *msg, ZC_U8 *buf, ZC_U32 buflen, ZC_U16 readlen, ReadCb readcb);

void zc_binmsg_debug_printhdr(const zc_binmsg_t *msg);
void zc_binmsg_debug_dump(zc_binmsg_t *msg);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_BINMSG_H__*/
