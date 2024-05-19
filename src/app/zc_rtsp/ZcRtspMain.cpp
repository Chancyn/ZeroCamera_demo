// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

#include "zc_log.h"

#include "ZcRtspManager.hpp"

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_rtsp.log"

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

int main(int argc, char **argv) {
    printf("main into\n");
    InitSignals();
    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    zc::CRtspManager rtsp;
    rtsp.Init();
    rtsp.Start();
    while (!bExitFlag) {
        sleep(1);
        LOG_DEBUG("sleep ");
    }

    LOG_ERROR("app loop exit");
    rtsp.Stop();
    rtsp.UnInit();
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
