// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zc_log.h"

#include "ZcRtmpPushMan.hpp"

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_rtsppushcli.log"

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
    if (argc < 2) {
        LOG_ERROR("args error pls ./zc_rtmppush url chn\n \
        example./zc_rtmppush rtmp://192.168.1.66:1935/live/push.ch0 0\n \
        example./zc_rtmppush rtmp://192.168.1.66:1935/live/push.ch1 1");
        return -1;
    }

    int chn = 0;
    int type = 0;
    if (argc > 2) {
        chn = atoi(argv[2]);
    }

    LOG_TRACE("pushcli url[%s] chn[%d] transport[%d]", argv[1], chn);
    zc::CRtmpPushMan cli;
    if (!cli.Init(type, chn, argv[1])) {
        goto _err;
    }

    cli.Start();
    while (!bExitFlag) {
        usleep(100 * 1000);
        // LOG_DEBUG("sleep ");
    }

    LOG_ERROR("app loop exit");
    cli.Stop();
    cli.UnInit();
_err:
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
