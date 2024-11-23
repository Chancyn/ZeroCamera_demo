// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "ZcIRtmp.hpp"
#include "ZcRtmpPullAioCli.hpp"
#include "ZcRtmpPullCli.hpp"
#include <stdint.h>

namespace zc {
// modid
typedef enum {
    ZC_RTMPPULL_E = 0,  // rtmppush cli
    ZC_RTMPPULL_AIO_E,  // rtmppushaio cli

    ZC_RTMPPULL_BUTT,  // end
} zc_rtmppull_type_e;

class CIRtmpPullFac {
 public:
    CIRtmpPullFac() {}
    ~CIRtmpPullFac() {}
    static CIRtmpPull *CreateRtmpPull(zc_rtmppull_type_e etype) {
        CIRtmpPull *push = nullptr;
        switch (etype) {
        case ZC_RTMPPULL_E:
            push = new CRtmpPullCli();
            break;
        case ZC_RTMPPULL_AIO_E:
            push = new CRtmpPullAioCli();
            break;
        default:
            LOG_ERROR("error, etype[%d]", etype);
            break;
        }
        return push;
    }
};

}  // namespace zc
