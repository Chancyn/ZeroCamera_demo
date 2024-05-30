// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include <string>

#include "media-source.h"

#include "ZcMediaTrack.hpp"
#include "ZcShmFIFO.hpp"

namespace zc {

class CMediaTrackAAC : public CMediaTrack {
    typedef struct _audio_info_ {
        int channels;     ///< number of audio channels
        int sample_bits;  ///< bits per sample
        int sample_rate;  ///< samples per second(frequency)
    } audio_info_t;

 public:
    explicit CMediaTrackAAC(int chn);
    virtual ~CMediaTrackAAC();
    virtual bool Init(void *info = nullptr);

 private:
    audio_info_t m_meidainfo;
};

}  // namespace zc
