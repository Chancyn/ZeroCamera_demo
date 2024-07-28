// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstddef>
#include <stdint.h>
#include <string.h>
#include <vector>

#include <string>

#include "NonCopyable.hpp"
#include "fmp4-writer.h"
#include "mov-buffer.h"
#include "zc_basic_fun.h"
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcShmStream.hpp"
#include "ZcMovBuf.hpp"

namespace zc {
typedef int (*OnFmp4PacketCb)(void *param, int type, const void *data, size_t bytes, uint32_t timestamp);

typedef struct {
    zc_movio_info_t movinfo;
    zc_stream_info_t streaminfo;
    OnFmp4PacketCb onfmp4packetcb;
    void *Context;
} zc_fmp4muxer_info_t;

class CFmp4Muxer : protected Thread {
    enum fmp4_status_e {
        fmp4_status_err = -1,  // error
        fmp4_status_init = 0,  // init
        fmp4_status_run,       // running
    };

 public:
    CFmp4Muxer();
    virtual ~CFmp4Muxer();

 public:
    bool Create(const zc_fmp4muxer_info_t &info);
    bool Destroy();
    bool Start();
    bool Stop();
    fmp4_status_e GetStatus() { return m_status; }

 private:
    bool destroyStream();
    bool createStream();
    int _write2Fmp4(zc_frame_t *pframe);
    int _getDate2Write2Fmp4(CShmStreamR *stream);
    int _packetProcess();
    virtual int process();

 private:
    bool m_Idr;
    fmp4_status_e m_status;
    fmp4_movio_e m_type;
    ZC_U64 m_pts;
    ZC_U64 m_apts;  // audio pts
    unsigned char m_framebuf[ZC_STREAM_MAXFRAME_SIZE];
    unsigned char m_framemp4buf[ZC_STREAM_MAXFRAME_SIZE];  // TODO + hdr
    zc_fmp4muxer_info_t m_info;
    CMovIo *m_movio;
    struct fmp4_writer_t *m_fmp4;
    int m_trackid[ZC_MEDIA_TRACK_BUTT];
    std::vector<CShmStreamR *> m_vector;
};

}  // namespace zc
