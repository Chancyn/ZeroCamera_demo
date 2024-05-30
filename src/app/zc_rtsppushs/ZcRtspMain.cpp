// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "zc_log.h"

#include "ZcRtspPushServer.hpp"

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_rtspcli.log"

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

    zc::CRtspPushServer svr;
    if (!svr.Init()) {
        LOG_TRACE("CRtspPushServer Init error");
        goto _done;
    }

    svr.Start();
    while (!bExitFlag) {
        usleep(100*1000);
        // LOG_DEBUG("sleep ");
    }

    _done:
    svr.Stop();
    svr.UnInit();
    LOG_ERROR("app loop exit");

    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
