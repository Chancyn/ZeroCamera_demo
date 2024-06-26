// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_log.h"
#include "zc_msg.h"

#include "ZcModBase.hpp"
#include "rtsp/ZcModRtsp.hpp"
#include "sys/ZcModSys.hpp"

namespace zc {
class CModFac {
 public:
    CModFac() {}
    ~CModFac() {}
    CModBase *CreateMod(ZC_U8 modid) {
        CModBase *pmod = nullptr;
        switch (modid) {
        case ZC_MODID_SYS_E:
            pmod = new CModSys();
            break;
        case ZC_MODID_RTSP_E:
            pmod = new CModRtsp();
            break;
        default:
            LOG_ERROR("error, modid[%d]", modid);
            break;
        }
        return pmod;
    }
};

}  // namespace zc
