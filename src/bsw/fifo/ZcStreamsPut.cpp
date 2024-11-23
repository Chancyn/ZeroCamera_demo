// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <asm-generic/errno.h>
#include <cstdint>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <list>
#include <memory>
#include <mutex>

#include "zc_basic_fun.h"
#include "zc_h26x_sps_parse.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcStreamsPut.hpp"

#include "ZcType.hpp"


namespace zc {

typedef struct {
    CShmStreamW *fifow;
    unsigned int chn;
    unsigned int datalen;
    const uint8_t *dataptr;
} zc_write_frame_t;

static zc_shmstreams_info_t g_livemp4infodef = {
    {
        {0, ZC_STREAM_MAIN_VIDEO_SIZE, ZC_STREAM_VIDEOPULL_SHM_PATH},
        {0, ZC_STREAM_AUDIO_SIZE, ZC_STREAM_AUDIOPULL_SHM_PATH},
        {0, ZC_STREAM_META_SIZE, ZC_STREAM_METAPULL_SHM_PATH},
    },
};

CStreamsPut::CStreamsPut()
    : m_status(0), m_chn(0) {
    memset(&m_info, 0, sizeof(m_info));
}

CStreamsPut::~CStreamsPut() {
    UnInit();
}

unsigned int CStreamsPut::_putingCb(void *stream) {
    zc_write_frame_t *writer = reinterpret_cast<zc_write_frame_t *>(stream);
    return writer->fifow->PutAppending(writer->dataptr, writer->datalen);
}

unsigned int CStreamsPut::putingCb(void *u, void *stream) {
    CStreamsPut *self = reinterpret_cast<CStreamsPut *>(u);
    return self->_putingCb(stream);
}

bool CStreamsPut::Init(int chn) {
    LOG_TRACE("Init into");
    m_chn = chn;
    memcpy(&m_info, &g_livemp4infodef, sizeof(m_info));
    for (unsigned int i = 0; i < _SIZEOFTAB(m_info.stream); i++) {
        m_info.stream[i].chn = m_chn;
    }

    for (unsigned int i = 0; i < ZC_MEDIA_TRACK_BUTT; i++) {
        m_fifowriter[i] =
            new CShmStreamW(m_info.stream[i].size, m_info.stream[i].fifopath, m_info.stream[i].chn, putingCb, this);
        if (!m_fifowriter[i]->ShmAlloc()) {
            LOG_ERROR("ShmAllocWrite error");
            ZC_ASSERT(0);
            // return -1;
        }
    }

    LOG_TRACE("Init OK");
    return true;
}

bool CStreamsPut::UnInit() {
    for (unsigned int i = 0; i < ZC_MEDIA_TRACK_BUTT; i++) {
        ZC_SAFE_DELETE(m_fifowriter[i]);
    }
    return true;
}

int CStreamsPut::PutFrame(zc_frame_t *framehdr, const uint8_t *data) {
    CShmStreamW *fifowriter = m_fifowriter[framehdr->type % ZC_STREAM_BUTT];
    if (fifowriter) {
#if 1  // dump
        if (framehdr->keyflag) {
            LOG_TRACE("stream:%d, encode:%d, seq[%d] put IDR:%d len:%d, pts:%u,utc:%u, wh[%u*%u]", framehdr->type,
                        framehdr->video.encode, framehdr->seq, framehdr->keyflag, framehdr->size, framehdr->pts, framehdr->utc,
                        framehdr->video.width, framehdr->video.height);
        }
#endif
        zc_write_frame_t framedata;
        framedata.fifow = fifowriter;
        framedata.dataptr = data;
        framedata.datalen = framehdr->size;
        fifowriter->Put((const unsigned char *)framehdr, sizeof(zc_frame_t), &framedata);
        return 0;
    }

    return -1;
}

}  // namespace zc
