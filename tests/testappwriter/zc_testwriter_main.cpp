// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zc_log.h"

#include "ZcTestWriterMan.hpp"

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

class CTestWriterMan {
 public:
    CTestWriterMan() {}
    virtual ~CTestWriterMan() {}
    bool Init() {}
    bool UnInit() {}

 private:
};

static int praseEncodetrans2Type(const char *encoding) {
    if (0 == strcasecmp("H264", encoding)) {
        return ZC_FRAME_ENC_H264;
    } else if (0 == strcasecmp("H265", encoding)) {
        return ZC_FRAME_ENC_H265;
    }
#if 0
    else if (0 == strcasecmp("AAC", encoding)) {
        return ZC_FRAME_ENC_AAC;
    }
#endif

    return ZC_FRAME_ENC_H264;
}

int main(int argc, char **argv) {
    printf("main into\n");
    InitSignals();
    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    unsigned int codeTab[ZC_STREAM_VIDEO_MAX_CHN] = {
        ZC_FRAME_ENC_H265,
        ZC_FRAME_ENC_H264,
    };

    int len = _SIZEOFTAB(codeTab);
    for (int i = 0; i < ZC_STREAM_VIDEO_MAX_CHN; i++) {
        if (argc > i + 1) {
            codeTab[i] = praseEncodetrans2Type(argv[i + 1]);
            LOG_TRACE("prase chn:%d, encode:%u, %s", i, codeTab[i], argv[i + 1]);
        } else {
            break;
        }
    }

    zc::CTestWriterMan man;
    if (man.Init(codeTab, len) < 0) {
        LOG_ERROR("error Init");
        goto _err;
    }
    man.Start();
    while (!bExitFlag) {
        sleep(1);
        // LOG_DEBUG("sleep exit");
    }
    LOG_ERROR("app loop exit");
    man.Stop();
    man.UnInit();
_err:
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
