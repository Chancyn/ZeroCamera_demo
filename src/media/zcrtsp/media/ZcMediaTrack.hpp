// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <memory>
#include <string>

#include "media-source.h"
#include "zc_frame.h"

#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"
#include "ZcType.hpp"

#define ZC_DEBUG_MEDIATRACK 1

namespace zc {
typedef enum {
    MEDIA_TRACK_VIDEO = 0,  // a=control:video
    MEDIA_TRACK_AUDIO = 1,  // a=control:audio
    MEDIA_TRACK_META = 2,   // a=control:meta

    MEDIA_TRACK_BUTT,
} media_track_e;

// video
typedef enum {
    // video
    MEDIA_CODE_H264 = 0,
    MEDIA_CODE_H265 = 1,

    // audio
    MEDIA_CODE_AAC = 10,

    // metadata
    MEDIA_CODE_METADATA = 20,

    MEDIA_CODE_BUTT,
} media_code_e;

class CLiveSource;
class CMediaTrack {
 public:
    explicit CMediaTrack(media_track_e track, int code);
    virtual ~CMediaTrack();
    virtual bool Init(void *info = nullptr) = 0;
    virtual void UnInit();

    int SetTransport(std::shared_ptr<IRTPTransport> transport);
    int SendBye();
    int SendRTCP(uint64_t clock);
    int GetSDPMedia(std::string &sdp) const;
    int GetRTPInfo(const char *uri, char *rtpinfo, size_t bytes) const;

    // CShmFIFOR *GetShmFIFOR() { return m_fiforeader; }
    // CShmStreamR *GetShmStreamR() { return m_fiforeader; }
    int GetEvFd() {
        if (!m_fiforeader) {
            return -1;
        }
        return m_fiforeader->GetEvFd();
    }
    // Get datafrom fifo and send
    int GetData2Send();

    static void OnRTCPEvent(void *param, const struct rtcp_msg_t *msg);
    static void *RTPAlloc(void *param, int bytes);
    static void RTPFree(void *param, void *packet);
    static int RTPPacket(void *param, const void *packet, int bytes, uint32_t timestamp, int flags);

 private:
    void _onRTCPEvent(const struct rtcp_msg_t *msg);
    void *_RTPAlloc(int bytes);
    void _RTPFree(void *packet);
    int _RTPPacket(const void *packet, int bytes, uint32_t timestamp, int flags);

 protected:
    bool m_create;  // create ok
    int m_track;    // trackid
    int m_code;     // codectype
    //CShmFIFOR *m_fiforeader;
    CShmStreamR *m_fiforeader;
    void *m_rtppacker;  // rtp encoder
    void *m_rtp;        // rtp status
    std::string m_sdp;
    unsigned int m_frequency;
    uint32_t m_timestamp;

 private:
    // CLiveSource *m_live;
    std::shared_ptr<IRTPTransport> m_transport;
    int m_evfd;
    int m_sendcnt;
    int m_pollcnt;
    uint64_t m_rtp_clock;
    uint64_t m_rtcp_clock;
    int64_t m_dts_first; // first frame timestamp
	int64_t m_dts_last; // last frame timestamp

    unsigned char m_packet[MAX_UDP_PACKET + 14];
    unsigned char m_framebuf[ZC_STREAM_MAXFRAME_SIZE];     // framebuf TODO(zhoucc): different size new
#if ZC_DEBUG_MEDIATRACK
    uint64_t m_debug_cnt_lasttime;
    uint32_t m_debug_framecnt_last;
    uint32_t m_debug_framecnt;
#endif
};
}  // namespace zc