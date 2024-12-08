// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>

#include <memory>
#include <string>

#include "zc_frame.h"

#include "Singleton.hpp"
#include "Thread.hpp"
#include "ZcShmFIFO.hpp"
#include "ZcShmStream.hpp"

class CMediaTrack;
namespace zc {
typedef struct {
    unsigned int chn;
    unsigned int size;
    char fifopath[128];
} zc_shmstreams_t;

typedef struct {
    zc_stream_info_t streaminfo;
} zc_shmstreams_getinfo_t;
int GetFrameCb(void *ctx, zc_frame_t *framehdr, const uint8_t *data);

class CStreamsGet: protected Thread  {
    enum get_status_e {
        get_status_err = -1,  // error
        get_status_init = 0,  // init
        get_status_run,       // running
    };

 public:
    CStreamsGet();
    virtual ~CStreamsGet();

 public:
    bool Init(int chn);
    bool UnInit();
 private:
    bool createStream();
    bool destroyStream();
    int _getFrameData(CShmStreamR *stream);
    int _packetProcess();
    virtual int process();

 private:
    int m_status;
    int m_chn;
    bool m_Idr;
    unsigned char m_framebuf[ZC_STREAM_MAXFRAME_SIZE];
    zc_shmstreams_getinfo_t m_info;
    CShmStreamR *m_fiforeader[ZC_MEDIA_TRACK_BUTT];
    int m_trackid[ZC_MEDIA_TRACK_BUTT];
    zc_frame_userinfo_t m_frameinfoTab[ZC_MEDIA_TRACK_BUTT];
};

}  // namespace zc
