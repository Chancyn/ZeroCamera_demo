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
    CMediaTrack *CreateMediaTrack(const zc_meida_track_t &info) {
        CMediaTrack *track = nullptr;
        LOG_TRACE("CreateMediaTrack into, code:%d, chn:%u, trackno:%u,", info.mediacode, info.chn, info.trackno);
        switch (info.mediacode) {
        case ZC_MEDIA_CODE_H264:
            track = new CMediaTrackH264(info);
            break;
        case ZC_MEDIA_CODE_H265:
            track = new CMediaTrackH265(info);
            break;
        case ZC_MEDIA_CODE_AAC:
            track = new CMediaTrackAAC(info);
            break;
        // case ZC_MEDIA_CODE_METADATA:
        // track = new CMediaTrackAAC();
        // break;
        default:
            LOG_ERROR("error, code[%d]", info.mediacode);
            break;
        }
        return track;
    }
};
}  // namespace zc
