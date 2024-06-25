// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zc_log.h"

#include "ZcSysManager.hpp"
#include "ZcStreamMgr.hpp"

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_sys.log"

static BOOL bExitFlag = FALSE;

static void SigProc(int dummy) {
    LOG_CRITI("\nReceived Signal %d\n", dummy);
    bExitFlag = TRUE;
}

static void InitSignals() {
    signal(SIGINT, SigProc);
    signal(SIGQUIT, SigProc);
    signal(SIGKILL, SigProc);
    signal(SIGTERM, SigProc);
    signal(SIGPIPE, SIG_IGN);
}

// streamMgr handle mod msg callback
static int StreamMgrHandleMsg(void *ptr, unsigned int type, void *indata, void *outdata) {
    LOG_ERROR("StreamMgrCb ptr:%p, type:%d, indata:%d", ptr, type);
    if (type == 0) {
        // TODO(zhoucc):
    }

    return -1;
}

// SysManager handle mod msg callback
static int SysMgrHandleMsg(void *ptr, unsigned int type, void *indata, void *outdata) {
    LOG_ERROR("SysMgrCb ptr:%p, type:%d, indata:%d", ptr, type);
    if (type == 0) {
        // TODO(zhoucc):
    }

    return -1;
}

int main(int argc, char **argv) {
    printf("main into\n");
    InitSignals();
    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    LOG_DEBUG("sizeof(void*)=%zu", sizeof(void *));
    zc_stream_mgr_cfg_t cfg;    // TODO(zhoucc): config
    g_ZCStreamMgrInstance.Init(NULL);
    zc::CSysManager sys;
    zc::sys_callback_info_t cbinfo = {
        .streamMgrHandleCb = StreamMgrHandleMsg,
        .streamMgrContext = nullptr,
        .MgrHandleCb = SysMgrHandleMsg,
        .MgrContext = &sys,
    };

    sys.Init(&cbinfo);
    sys.Start();
    while (!bExitFlag) {
        usleep(100*1000);
        // LOG_TRACE("sleep ");
    }

    LOG_ERROR("app loop exit");
    sys.Stop();
    sys.UnInit();
    g_ZCStreamMgrInstance.UnInit();
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
