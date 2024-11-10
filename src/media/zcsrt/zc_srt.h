// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_SRT_H__
#define __ZC_SRT_H__

#include <srt/srt.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* This is for MPEG-TS and it's a default SRTO_PAYLOADSIZE for SRTT_LIVE (8 TS packets) */
#ifndef SRT_LIVE_DEFAULT_PAYLOAD_SIZE
#define SRT_LIVE_DEFAULT_PAYLOAD_SIZE 1316
#endif

/* This is the maximum payload size for Live mode, should you have a different payload type than MPEG-TS */
#ifndef SRT_LIVE_MAX_PAYLOAD_SIZE
#define SRT_LIVE_MAX_PAYLOAD_SIZE 1456
#endif

#define SRT_POLLING_TIME 100    /// Time in milliseconds between interrupt check


/**
 * @name URL open modes
 * The flags argument to avio_open must be one of the following
 * constants, optionally ORed with other flags.
 * @{
 */
#define ZC_AVIO_FLAG_READ  1                                      /**< read-only */
#define ZC_AVIO_FLAG_WRITE 2                                      /**< write-only */
#define ZC_AVIO_FLAG_READ_WRITE (AVIO_FLAG_READ|AVIO_FLAG_WRITE)  /**< read-write pseudo flag */

/**
 * Use non-blocking mode.
 * If this flag is set, operations on the context will return
 * AVERROR(EAGAIN) if they can not be performed immediately.
 * If this flag is not set, operations on the context will never return
 * AVERROR(EAGAIN).
 * Note that this flag does not affect the opening/connecting of the
 * context. Connecting a protocol will always block if necessary (e.g. on
 * network protocols) but never hang (e.g. on busy devices).
 * Warning: non-blocking protocols is work-in-progress; this flag may be
 * silently ignored.
 */
#define ZC_AVIO_FLAG_NONBLOCK 8

typedef enum {
    SRT_MODE_CALLER = 0,     // Caller
    SRT_MODE_LISTENER = 1,   // Listener
    SRT_MODE_RENDEZVOUS = 2  // Rendezous
} SRTMode;

typedef struct {
    uint8_t srtmode;
    uint16_t port;
    char proto[32];
    char hostname[256];
    char path[256];
    char uri[1024];
} zc_srt_url_t;

typedef struct {
    int flags;
    int max_packet_size;        /**< if non zero, the stream is packetized with this max packet size */
    int is_streamed;            /**< true if streamed (no seek possible), default = false */
    int is_connected;
} zc_srt_flags_t;

typedef struct _SRTContext {
    int fd;
    int eid;
    int64_t rw_timeout;
    int64_t listen_timeout;
    int recv_buffer_size;
    int send_buffer_size;

    int64_t maxbw;
    int pbkeylen;
    char *passphrase;
#if SRT_VERSION_VALUE >= 0x010302
    int enforced_encryption;
    int kmrefreshrate;
    int kmpreannounce;
    int64_t snddropdelay;
#endif
    int mss;
    int ffs;
    int ipttl;
    int iptos;
    int64_t inputbw;
    int oheadbw;
    int64_t latency;
    int tlpktdrop;
    int nakreport;
    int64_t connect_timeout;
    int payload_size;
    int64_t rcvlatency;
    int64_t peerlatency;
    SRTMode mode;
    int sndbuf;
    int rcvbuf;
    int lossmaxttl;
    int minversion;
    char *streamid;
    char *smoother;
    int messageapi;
    SRT_TRANSTYPE transtype;
    int linger;
    int tsbpd;
} SRTContext;

int64_t av_gettime_relative(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif      // #ifndef __ZC_SRT_H__


