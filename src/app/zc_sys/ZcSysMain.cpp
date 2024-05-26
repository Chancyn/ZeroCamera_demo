// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "zc_log.h"

#include "ZcSysManager.hpp"

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

int main(int argc, char **argv) {
    printf("main into\n");
    InitSignals();
    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    LOG_DEBUG("sizeof(void*)=%zu", sizeof(void *));
    zc::CSysManager sys;
    sys.Init();
    sys.Start();
    while (!bExitFlag) {
        sleep(1);
        // LOG_TRACE("sleep ");
    }

    LOG_ERROR("app loop exit");
    sys.Stop();
    sys.UnInit();
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
