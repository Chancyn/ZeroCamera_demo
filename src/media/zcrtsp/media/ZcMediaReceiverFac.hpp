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
    CMediaReceiver *CreateMediaReceiver(int code, int shmtype, int chn) {
        CMediaReceiver *recv = nullptr;
        LOG_ERROR("into, code:%d, shmtype:%d, chn:%d", code, shmtype, chn);
        switch (code) {
        case ZC_MEDIA_CODE_H264:
            recv = new CMediaReceiverH264(shmtype, chn, ZC_STREAM_MAXFRAME_SIZE);
            break;
        case ZC_MEDIA_CODE_H265:
            recv = new CMediaReceiverH265(shmtype, chn, ZC_STREAM_MAXFRAME_SIZE);
            break;
        case ZC_MEDIA_CODE_AAC:
            recv = new CMediaReceiverAAC(shmtype, chn);
            break;
        default:
            LOG_ERROR("error, code[%d]", code);
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
