// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "zc_log.h"

#include "ZcRtspPushClient.hpp"

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
        LOG_ERROR("args error pls ./zc_rtsppushcli url chn [tcp/udp]\n \
        example./zc_rtsppushcli rtsp://192.168.1.66:8554/live/push.ch0 0 tcp\n \
        example./zc_rtsppushcli rtsp://192.168.1.66:8554/live/push.ch1 1 udp");
        return -1;
    }

    int chn = 0;
    int transport = zc::ZC_RTSP_TRANSPORT_RTP_TCP;
    if (argc > 2) {
        chn = atoi(argv[2]);
    }

    if (argc > 3) {
        if (strncasecmp(argv[3], "udp", strlen("udp")) == 0) {
            transport = zc::ZC_RTSP_TRANSPORT_RTP_UDP;
        }
    }

    LOG_TRACE("pushcli url[%s] chn[%d] transport[%d]", argv[1], chn, transport);
    zc::CRtspPushClient cli{argv[1], chn, transport};

    cli.StartCli();
    while (!bExitFlag) {
        usleep(100 * 1000);
        // LOG_DEBUG("sleep ");
    }

    LOG_ERROR("app loop exit");
    cli.StopCli();
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
