// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include <vector>

#include "flv-muxer.h"
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcShmStream.hpp"
#include "ZcStreamTrace.hpp"

namespace zc {
typedef int (*OnFlvPacketCb)(void *param, int type, const void *data, size_t bytes, uint32_t timestamp);

// FLV Tag Type
enum flv_tag_type_e {
    flv_tag_audio = 8,    // FLV_TYPE_AUDIO
    flv_tag_video = 9,    // FLV_TYPE_VIDEO
    flv_tag_script = 18,  // FLV_TYPE_SCRIPT
};

typedef struct {
    zc_stream_info_t streaminfo;
    OnFlvPacketCb onflvpacketcb;
    void *Context;
} zc_flvmuxer_info_t;

class CFlvMuxer : protected Thread {
    enum flv_status_e {
        flv_status_err = -1,  // error
        flv_status_init = 0,  // init
        flv_status_run,       // running
    };

 public:
    CFlvMuxer();
    virtual ~CFlvMuxer();

 public:
    bool Create(const zc_flvmuxer_info_t &info);
    bool Destroy();
    bool Start();
    bool Stop();
    flv_status_e GetStatus() { return m_status; }

 private:
    bool destroyStream();
    bool createStream();
    int _fillFlvMuxerMeta();
    int _packetFlv(zc_frame_t *frame);
    int _getDate2PacketFlv(CShmStreamR *stream);
    int _packetProcess();
    virtual int process();

 private:
    bool m_Idr;
    flv_status_e m_status;
    ZC_U64 m_pts;
    ZC_U64 m_apts;  // audio pts
    unsigned char m_framebuf[ZC_STREAM_MAXFRAME_SIZE];
    zc_flvmuxer_info_t m_info;
    flv_muxer_t *m_flv;
    zc_frame_userinfo_t m_frameinfoTab[ZC_MEDIA_TRACK_BUTT];
    std::vector<CShmStreamR *> m_vector;

    CStreamTrace m_trace;
};

}  // namespace zc
