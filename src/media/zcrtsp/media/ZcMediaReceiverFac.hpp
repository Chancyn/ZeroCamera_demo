// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "media-source.h"
#include <memory>
#include <stdint.h>
#include <string>

#include "ZcMediaReceiver.hpp"
#include "ZcMediaReceiverAAC.hpp"
#include "ZcMediaReceiverH264.hpp"
#include "ZcMediaReceiverH265.hpp"
#include "ZcType.hpp"
#include "zc_log.h"

namespace zc {
class CMediaReceiverFac {
 public:
    CMediaReceiverFac() {}
    ~CMediaReceiverFac() {}
    CMediaReceiver *CreateMediaReceiver(const zc_meida_track_t &info) {
        CMediaReceiver *recv = nullptr;
        LOG_TRACE("into, code:%u, chn:%u, trackno:%u, tracktype:%u, shm:%s", info.mediacode, info.chn, info.trackno,
                  info.tracktype, info.name);
        switch (info.mediacode) {
        case ZC_MEDIA_CODE_H264:
            recv = new CMediaReceiverH264(info);
            break;
        case ZC_MEDIA_CODE_H265:
            recv = new CMediaReceiverH265(info);
            break;
        case ZC_MEDIA_CODE_AAC:
            recv = new CMediaReceiverAAC(info);
            break;
        default:
        LOG_ERROR("error, code:%u, chn:%u, trackno:%u, tracktype:%u, shm:%s", info.mediacode, info.chn, info.trackno,
                  info.tracktype, info.name);
            break;
        }
        return recv;
    }
    // std::shared_ptr<CMediaReceiver> MakesharedMediaReceiver(int code, shmtype, int chn) {
    //     LOG_ERROR("into, code[%d]", code);
    //     if (ZC_MEDIA_CODE_H264 == code) {
    //         return std::make_shared<CMediaReceiver>(new CMediaReceiverH264(shmtype, chn, ZC_STREAM_MAXFRAME_SIZE));
    //     } else if (ZC_MEDIA_CODE_H265 == code) {
    //         return std::make_shared<CMediaReceiver>(new CMediaReceiverH265(shmtype, chn, ZC_STREAM_MAXFRAME_SIZE));
    //     } else if (ZC_MEDIA_CODE_AAC == code) {
    //         return std::make_shared<CMediaReceiver>(new CMediaReceiverAAC(shmtype, chn));
    //     } else {
    //         LOG_ERROR("error code[%d]", code);
    //         return nullptr;
    //     }
    // }
};
}  // namespace zc
