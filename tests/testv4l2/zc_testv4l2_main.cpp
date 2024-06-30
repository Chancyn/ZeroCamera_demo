// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ZcFFEncoder.hpp"
#include "zc_log.h"

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_testv4l2.log"

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

#if defined(WITH_FFMPEG)
int main(int argc, char **argv) {
    printf("main into\n");
    InitSignals();

    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    if (argc < 3) {
        LOG_ERROR("pls input args ./test wxh fps [codetype]");
        LOG_ERROR("such as ./test 640x480 30 H264");
        LOG_ERROR("such as ./test 1280x720 25 H265");
        return -1;
    }
    char res[32] = "1920x1080";
    int fps = 30;
    int width = 1920;
    int height = 1080;

    strncpy(res, argv[1], sizeof(res));
    char *delim = nullptr;
    char *pheight = nullptr;
    if ((delim = strchr(res, 'x')) != nullptr || (delim = strchr(res, 'X')) != nullptr ||
        (delim = strchr(res, '*')) != nullptr) {
        *delim++ = '\0';
        width = atoi(res);
        height = atoi(delim);
        if (width <= 0 || height <= 0) {
            width = 640;
            height = 480;
        }
    }

    fps = atoi(argv[2]);
    if (fps <= 0) {
        fps = 30;
    }

    int codectype = ZC_FRAME_ENC_H264;
    if (argc > 3) {
        if (strncasecmp(argv[3], "h264", 4) == 0) {
            codectype = ZC_FRAME_ENC_H264;
        } else if (strncasecmp(argv[3], "h265", 4) == 0 || strncasecmp(argv[3], "hevc", 4) == 0) {
            codectype = ZC_FRAME_ENC_H265;
        }
    }
    LOG_DEBUG("INTO wh:%dx%d, fps:%d, code:%d", width, height, fps, codectype);
    zc::zc_ffcodec_info_t info{
        codectype,
        fps,
        width,
        height,
    };
    zc::CFFEncoder encoder{"/dev/video0", info};
    if (!encoder.Open()) {
        LOG_ERROR("open error");
    }

    while (!bExitFlag) {
        sleep(1);
        // LOG_DEBUG("sleep exit");
    }
    encoder.Close();
    LOG_ERROR("app loop exit");

    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
#else
int main(int argc, char **argv) {
    printf("main exit, not with ffmpeg\n");
    return 0;
}
#endif
