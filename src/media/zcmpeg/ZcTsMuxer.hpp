// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include <vector>

#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcShmStream.hpp"
#include "ZcStreamTrace.hpp"

namespace zc {
typedef int (*OnTsPacketCb)(void *param, const void *data, size_t bytes);

#define ZC_N_TS_PACKET 188

typedef struct {
    zc_stream_info_t streaminfo;
    OnTsPacketCb onTsPacketCb;
    void *Context;
} zc_tsmuxer_info_t;

class CTsMuxer : protected Thread {
    enum ts_status_e {
        ts_status_err = -1,  // error
        ts_status_init = 0,  // init
        ts_status_run,       // running
    };

 public:
    CTsMuxer();
    virtual ~CTsMuxer();

 public:
    bool Create(const zc_tsmuxer_info_t &info);
    bool Destroy();
    bool Start();
    bool Stop();
    ts_status_e GetStatus() { return m_status; }

 private:
    bool destroyStream();
    bool createStream();
    int _tsAddStream(zc_frame_t *frame);
    int _checkPts(zc_frame_t *frame);
    int _packetTs(zc_frame_t *frame);
    int _getDate2PacketTs(CShmStreamR *stream);
    int _packetProcess();
    virtual int process();
    static void* tsAlloc(void *ptr, uint64_t bytes);
    void* _tsAlloc(uint64_t bytes);
    static int tsWrite(void* ptr, const void* packet, size_t bytes);
    int _tsWrite(const void* packet, size_t bytes);
    static void tsFree(void *ptr, void* packet);
    void _tsFree(void* packet);

 private:
    bool m_Idr;
    ts_status_e m_status;
    ZC_U64 m_pts[ZC_STREAM_BUTT];
    int m_streamid[ZC_STREAM_BUTT];
    unsigned char m_framebuf[ZC_STREAM_MAXFRAME_SIZE];
    unsigned char m_pkgbuf[ZC_N_TS_PACKET];
    zc_tsmuxer_info_t m_info;
    void *m_ts;
    zc_frame_userinfo_t m_frameinfoTab[ZC_STREAM_BUTT];
    std::vector<CShmStreamR *> m_vector;

    CStreamTrace m_trace;
};

}  // namespace zc
