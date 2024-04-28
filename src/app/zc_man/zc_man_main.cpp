// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ZCNngRepServer.hpp"
#include "ZCNngReqClient.hpp"
#include "zc_log.h"

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_man.log"

#define NODE0_URL "tcp://127.0.0.1:5555"
#define NODE1_URL "tcp://127.0.0.1:5555"
#define DATE "DATE"

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

// using namespace zc;

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

char *date(void) {
    static char buffer[128];
    time_t now = time(&now);
    struct tm info;
    localtime_r(&now, &info);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &info);
    return (buffer);
}

static int zc_nng_msg_cbfun(char *in, size_t isize, char *out, size_t *osize) {
    LOG_DEBUG("msg_cbfun into");
    char *tmp = date();
    size_t len = strlen(tmp);
    strncpy(out, tmp, len);
    *osize = len + 1;
    LOG_DEBUG("msg_cbfun exit osize[%zu][%s]", *osize, out);
    return 0;
}

int main(int argc, char **argv) {
    printf("main into\n");
    InitSignals();
    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    LOG_DEBUG("app into");

    int nodetype = 0;
    if (argc > 1) {
        nodetype = atoi(argv[1]);
    }

    LOG_DEBUG("NODE type [%d]", nodetype);
    zc::NngRepServer ser;
    zc::NngReqClient cli;

    if (nodetype) {
        char msg[256] = DATE;
        LOG_DEBUG("NODE1 sendto NODE0");
        cli.Open(NODE0_URL);
        cli.Send(msg, strlen(msg) + 1, 0);
    } else {
        LOG_DEBUG("NODE0 start NODE1");
        ser.Open(NODE0_URL, zc_nng_msg_cbfun);
        ser.Start();
    }

    LOG_DEBUG("NODE0 sendto end");
    while (!bExitFlag) {
        sleep(1);
        LOG_DEBUG("sleep ");
    }

    ser.Stop();
    ser.Close();
    cli.Close();
    LOG_ERROR("app loop exit");
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
