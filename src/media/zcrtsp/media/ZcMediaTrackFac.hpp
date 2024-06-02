// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include <string>

#include "media-source.h"

#include "ZcMediaTrack.hpp"
#include "ZcMediaTrackAAC.hpp"
#include "ZcMediaTrackH264.hpp"
#include "ZcMediaTrackH265.hpp"
#include "ZcType.hpp"
#include "zc_log.h"

namespace zc {
class CMediaTrackFac {
 public:
    CMediaTrackFac() {}
    ~CMediaTrackFac() {}
    CMediaTrack *CreateMediaTrack(int code, int shmtype, int chn) {
        CMediaTrack *track = nullptr;
        LOG_ERROR("CreateMediaTrack into, code:%d, shmtype:%d, chn:%d", code, shmtype, chn);
        switch (code) {
        case ZC_MEDIA_CODE_H264:
            track = new CMediaTrackH264(shmtype, chn);
            break;
        case ZC_MEDIA_CODE_H265:
            track = new CMediaTrackH265(shmtype, chn);
            break;
        case ZC_MEDIA_CODE_AAC:
            track = new CMediaTrackAAC(shmtype, chn);
            break;
        // case ZC_MEDIA_CODE_AAC:
        // track = new CMediaTrackAAC();
        // break;
        default:
            LOG_ERROR("error, code[%d]", code);
            break;
        }
        return track;
    }
};
}  // namespace zc
