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
 public:
    explicit CMediaTrackAAC(const zc_meida_track_t &info);
    virtual ~CMediaTrackAAC();
    virtual bool Init(void *info = nullptr);
};

}  // namespace zc
