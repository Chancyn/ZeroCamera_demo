// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "zc_binmsg.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#if ZC_BINMSG_DEBUG
void zc_binmsg_debug_printhdr(const zc_binmsg_t *msg) {
    LOG_TRACE("binmsghdr hdrlen:%u,flags:[order:%d,crc:%d],id:%u,idto:%u,type:%u,\n \
    cmd:0x%04X,seq:%u,len:%u,crc32:0x%08X",
              sizeof(zc_binmsg_t), msg->flags.order, msg->flags.crc, msg->id, msg->idto, msg->msgtype, msg->cmd,
              msg->seq, msg->size, msg->crc32);

    if (msg->size > 0) {
        LOG_TRACE("\n binmsghdr string:%s", msg->data);
    }
    return 0;
}

void zc_binmsg_debug_dump(zc_binmsg_t *msg) {
    if (!msg) {
        return;
    }
    unsigned char *buf = msg;
    ZC_U16 cmd = msg->cmd;
    ZC_U16 size = msg->size;
    ZC_U16 seq = msg->seq;
    if (msg->flags.order) {
        size = ntohs(size);
        cmd = ntohs(cmd);
        seq = ntohs(seq);
    }
    LOG_TRACE("dump binmsg hdrlen:%u,flags:[order:%d,crc:%d],id:%u,idto:%u,type:%u,\n \
        cmd:0x%04X,seq:%u,len:%u,crc32:0x%08X",
              sizeof(zc_binmsg_t), msg->flags.order, msg->flags.crc, msg->id, msg->idto, msg->msgtype, cmd, seq, size,
              msg->crc32);
    for (int i = 0; i < sizeof(zc_binmsg_t) + size; i++) {
        printf("%02X ", buf[i]);
    }

    if (size > 0) {
        LOG_TRACE("\n dump binmsg string:%s", msg->data);
    }
    return;
}
#endif
void zc_binmsg_packhdr(zc_binmsg_t *msg, BOOL netorder, BOOL crc32, ZC_U8 id, ZC_U8 idto, ZC_U8 type, ZC_U16 cmd,
                       ZC_U16 seq) {
    ZC_ASSERT(msg != NULL);
    memset(msg, 0, sizeof(zc_binmsg_t));

    msg->magic[0] = ZC_BINMSG_MAGIC0;
    msg->magic[1] = ZC_BINMSG_MAGIC1;
    msg->flags.order = netorder;
    msg->flags.crc = crc32;
    msg->version = ZC_BINMSG_VERSION;
    msg->id = id;
    msg->idto = idto;
    msg->msgtype = type;

    if (!msg->flags.order) {
        msg->cmd = cmd;
        msg->seq = seq;
    } else {
        msg->cmd = htons(cmd);
        msg->seq = htons(seq);
    }

    return;
}

void zc_binmsg_packdata(zc_binmsg_t *msg, const ZC_U8 *data, ZC_U16 len) {
    ZC_ASSERT(msg != NULL);
    if (data == NULL) {
        len = 0;
    }

    if (!msg->flags.order) {
        msg->size = len;
    } else {
        msg->size = htonl(len);
    }

    if (len > 0) {
        memcpy(msg->data, data, len);
    }

    if (msg->flags.crc) {
        // TODO(zhoucc): crc32
    }

    return;
}

int zc_binmsg_parse(zc_binmsg_t *msg, ZC_U8 *buf, ZC_U32 buflen, ZC_U16 readlen, ReadCb readcb) {
    ZC_U16 datalen = 0;
    int remain = 0;
    int pos = 0;  // read pos
    zc_binmsg_t *pmsgtmp = NULL;
    ZC_U16 msglen = 0;
    BOOL bmsg = FALSE;

    // parse magic hdr
    while (buflen >= readlen) {
        // read hdr
        if (readlen < sizeof(zc_binmsg_t) + pos) {
            remain = sizeof(zc_binmsg_t) + pos - readlen;
            if (!readcb || remain != readcb(buf + pos + readlen, remain)) {
                LOG_ERROR("parse hdr error, read error len[%d]", remain);
                return -1;
            }
            readlen += remain;
        }

        if (*(buf + pos) != ZC_BINMSG_MAGIC0) {
            pos++;
            continue;
        }

        if (*(buf + pos + 1) != ZC_BINMSG_MAGIC1) {
            pos++;
            continue;
        }

        bmsg = TRUE;
        memcpy(msg, buf + pos, sizeof(zc_binmsg_t));

        if (!msg->flags.order) {
            msglen = msg->size;
        } else {
            msglen = ntohs(msg->size);
        }

        break;
    }

    if (!bmsg) {
        LOG_ERROR("parse msg error");
        return -1;
    }

    LOG_TRACE("parse msghdr ok, pos[%d], msglen:%u", pos, msglen);
    if (msglen > 0 && (readlen < sizeof(zc_binmsg_t) + pos + msglen)) {
        remain = sizeof(zc_binmsg_t) + msglen + pos - readlen;

        // buffer not enough,memove
        if (buflen < sizeof(zc_binmsg_t) + msglen + pos) {
            LOG_WARN("parse msg warning msglen+pos:%u+%u over %u buff memmove", msglen, pos, buflen);
            memmove(buf, buf + pos, readlen - pos);
            readlen -= pos;
            pos = 0;
        }

        if (!readcb || remain != readcb(buf + pos + readlen, remain)) {
            LOG_ERROR("parse msg pack error read remain:%d", remain);
            return -1;
        }
        readlen += remain;
    }

    // check crc32
    if (msg->flags.crc) {
        // TODO(zhoucc): crc32
    }

    // TODO(zhoucc): byte order
    if (msg->flags.order) {
        msg->cmd = ntohs(msg->cmd);
        msg->seq = ntohs(msg->seq);
        msg->crc32 = ntohl(msg->crc32);
    }

    return pos;
}

int zc_binmsg_unpackhdr(zc_binmsg_t *msg, const ZC_U8 *buf, ZC_U16 len) {
    ZC_ASSERT(msg != NULL);

    if (buf == NULL || len < sizeof(zc_binmsg_t)) {
        LOG_ERROR("unpack error len[%d]", len);
        return -1;
    }

    memcpy(msg, buf, sizeof(zc_binmsg_t));
    if (msg->magic[0] != ZC_BINMSG_MAGIC0 || msg->magic[1] != ZC_BINMSG_MAGIC1) {
        LOG_ERROR("unpack magic error[0x%02X,0x%02X]", msg->magic[0], msg->magic[1]);
        return -1;
    }

    // TODO(zhoucc): byte order
    if (msg->flags.order) {
        msg->cmd = ntohs(msg->cmd);
        msg->seq = ntohs(msg->seq);
        msg->crc32 = ntohl(msg->crc32);
    }

    return 0;
}