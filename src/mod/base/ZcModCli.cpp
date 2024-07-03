// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"
#include "zc_msg_codec.h"
#include "zc_msg_rtsp.h"
#include "zc_msg_sys.h"
#include "zc_proc.h"
#include "zc_type.h"
#include <functional>

#include "ZcModCli.hpp"
#include "ZcType.hpp"

// debug dump
#if ZC_DEBUG
#define ZC_DEBUG_DUMP 1
#else
#define ZC_DEBUG_DUMP 0
#endif

namespace zc {

CModCli::CModCli(ZC_U8 modid, ZC_U32 version) : CModReqCli(modid, version), CModSubscriber(ZC_MODID_SYS_E) {}

CModCli::~CModCli() {}

bool CModCli::initSubCli(MsgCommSubCliHandleCb handle) {
    if (!CModSubscriber::InitSub(handle)) {
        LOG_ERROR("initPubSvr error Open");
        return false;
    }

    if (!CModSubscriber::Start()) {
        LOG_ERROR("initPubSvr error start");
        return false;
    }
    LOG_TRACE("initPubSvr ok");
    return true;
}

bool CModCli::Init(MsgCommSubCliHandleCb handle) {
    initSubCli(handle);
    return true;
}

bool CModCli::unInitSubCli() {
    CModSubscriber::Stop();
    CModSubscriber::UnInitSub();

    return true;
}

bool CModCli::UnInit() {
    unInitSubCli();
    return true;
}

}  // namespace zc
