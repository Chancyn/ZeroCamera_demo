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
    zc_shmstreams_t stream[ZC_MEDIA_TRACK_BUTT];
} zc_shmstreams_info_t;

class CStreamsPut {
 public:
    CStreamsPut();
    virtual ~CStreamsPut();

 public:
    bool Init(int chn);
    bool UnInit();
    int PutFrame(zc_frame_t *framehdr, const uint8_t *data);
 private:
    static unsigned int putingCb(void *u, void *stream);
    unsigned int _putingCb(void *stream);
 private:
    int m_status;
    int m_chn;
    zc_shmstreams_info_t m_info;
    CShmStreamW *m_fifowriter[ZC_MEDIA_TRACK_BUTT];
};

}  // namespace zc
