// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "ZcIRtmp.hpp"
#include "ZcRtmpPushAioCli.hpp"
#include "ZcRtmpPushCli.hpp"
#include <stdint.h>

namespace zc {
// modid
typedef enum {
    ZC_RTMPPUSH_E = 0,  // rtmppush cli
    ZC_RTMPPUSH_AIO_E,  // rtmppushaio cli

    ZC_RTMPPUSH_BUTT,  // end
} zc_rtmppush_type_e;

class CIRtmpPushFac {
 public:
    CIRtmpPushFac() {}
    ~CIRtmpPushFac() {}
    static CIRtmpPush *CreateRtmpPush(zc_rtmppush_type_e etype) {
        CIRtmpPush *push = nullptr;
        switch (etype) {
        case ZC_RTMPPUSH_E:
            push = new CRtmpPushCli();
            break;
        case ZC_RTMPPUSH_AIO_E:
            push = new CRtmpPushAioCli();
            break;
        default:
            LOG_ERROR("error, etype[%d]", etype);
            break;
        }
        return push;
    }
};

}  // namespace zc
