// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include "Thread.hpp"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg_sys.h"

#include "mongoose.h"

#include "ZcType.hpp"
#include "ZcHttpFlvSess.hpp"
#include "ZcWebServer.hpp"

namespace zc {
#if ZC_SUPPORT_HTTP_FLV
int CHttpFlvSess::process() {
    LOG_WARN("process into\n");
    while (State() == Running /*&&  i < loopcnt*/) {
        mg_mgr_poll(mgr, 1000);
    }
    LOG_WARN("process exit\n");
    return -1;
}

CHttpFlvSess::CHttpFlvSess(const zc_flvmuxer_info_t &info) {
    return;
}

CHttpFlvSess::~CHttpFlvSess() {
    return;
}

int CHttpFlvSess::OnFlvPacketCb(void *param, int type, const void *data, size_t bytes, uint32_t timestamp)
{
    CHttpFlvSess *sess = reinterpret_cast<CHttpFlvSess *>(param);
    return sess->_onFlvPacketCb(type, data, bytes, timestamp);
}

int CHttpFlvSess::_onFlvPacketCb(int type, const void *data, size_t bytes, uint32_t timestamp)
{

}

//
void CHttpFlvSess::EventHandler() {
    return;
}
#endif

}  // namespace zc
