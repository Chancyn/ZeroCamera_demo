// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <functional>

#include "MsgCommRepServer.hpp"
#include "MsgCommReqClient.hpp"
#include "zc_log.h"
#include "zc_test_msgcomm.hpp"

#define NODE0_URL "tcp://127.0.0.1:5555"
#define NODE1_URL "tcp://127.0.0.1:5555"
#define DATE "DATE"

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

char *date(void) {
    static char buffer[128];
    time_t now = time(&now);
    struct tm info;
    localtime_r(&now, &info);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &info);
    return (buffer);
}

static int msgcomm_msg_cbfun(char *in, int iqsize, char *out, int *opsize) {
    LOG_DEBUG("msg_cbfun into");
    char *tmp = date();
    size_t len = strlen(tmp);
    strncpy(out, tmp, len);
    *opsize = len + 1;
    LOG_DEBUG("msg_cbfun exit opsize[%zu][%s]", *opsize, out);
    return 0;
}

static zc::CMsgCommRepServer g_ser;
static zc::CMsgCommReqClient g_cli;

// 0 server;1 client
int zc_test_msgcomm_start(int nodetype) {
    LOG_DEBUG("zc_test_msgcomm_start into");

    LOG_DEBUG("NODE type [%d]", nodetype);
    if (nodetype) {
        char msg[256] = DATE;
        LOG_DEBUG("NODE1 sendto NODE0");
        g_cli.Open(NODE0_URL);
        g_cli.Send(msg, strlen(msg) + 1, 0);
    } else {
        LOG_DEBUG("NODE0 start NODE1");
        // std::function<int(char*, int, char*, int*)>
        auto cb = std::bind(&msgcomm_msg_cbfun, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                            std::placeholders::_4);
        g_ser.Open(NODE0_URL, cb);
        // g_ser.Open(NODE0_URL, msgcomm_msg_cbfun);
        g_ser.Start();
    }

    LOG_DEBUG("NODE0 sendto end");

    return 0;
}

int zc_test_msgcomm_stop() {
    LOG_TRACE("zc_test_msgcomm_start exit");

    g_ser.Stop();
    g_ser.Close();
    g_cli.Close();
    LOG_TRACE("zc_test_msgcomm_stop exit");
    return 0;
}
