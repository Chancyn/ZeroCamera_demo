// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zc_log.h"
#if ZC_LIVE_TEST
#include "ZcLiveTestWriterSys.hpp"
#endif

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_tests.log"

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

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
#if ZC_LIVE_TEST
    g_ZCLiveTestWriterInstance.Init();
#endif

    while (!bExitFlag) {
        sleep(1);
        // LOG_DEBUG("sleep exit");
    }
    LOG_ERROR("app loop exit");
#if ZC_LIVE_TEST
    g_ZCLiveTestWriterInstance.UnInit();
#endif
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
