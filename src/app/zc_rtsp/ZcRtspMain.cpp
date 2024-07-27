// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zc_log.h"
#include "zc_proc.h"

#include "ZcRtspManager.hpp"
#include "ZcStreamMgrCli.hpp"

#if (ZC_LIVE_TEST && DZC_LIVE_TEST_THREADSHARED)
#include "ZcLiveTestWriterSys.hpp"
#endif

#define ZC_LOG_PATH "./log"
#define ZC_APP_NAME "zc_rtsp"
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

// streamMgr handle mod msg callback
static int StreamMgrHandleMsg(void *ptr, unsigned int type, void *indata, void *outdata) {
    // LOG_TRACE("StreamMgrCb ptr:%p, type:%d, indata:%d", ptr, type);
    if (type == 0) {
        // TODO(zhoucc):
    }

    return -1;
}

int main(int argc, char **argv) {
    ZC_PROC_SETNAME(ZC_APP_NAME);
    char pname[ZC_MAX_PNAME] = {0};
    ZC_PROC_GETNAME(pname, ZC_MAX_PNAME);

    InitSignals();
    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    LOG_INFO("process[%s,%s], build:[%s]\n",argv[0], pname, ZcGetBuildDateTimeStr());
#if (ZC_LIVE_TEST && DZC_LIVE_TEST_THREADSHARED)
#warning "zhoucc not process share testwrite"
    g_ZCLiveTestWriterInstance.Init();
#endif
    zc_streamcli_t cli = {0};
    cli.mod = ZC_MODID_RTSP_E;
    cli.pid = getpid();

    strncpy(cli.pname, pname, sizeof(cli.pname) - 1);
    g_ZCStreamMgrCliInstance.Init(&cli);
    zc::CRtspManager rtsp;

    zc::rtsp_callback_info_t cbinfo = {
        .streamMgrHandleCb = StreamMgrHandleMsg,
        .streamMgrContext = nullptr,
        .MgrHandleCb = zc::CRtspManager::RtspMgrHandleMsg,
        .MgrHandleSubCb = zc::CRtspManager::RtspMgrHandleSubMsg,
        .MgrContext = &rtsp,
    };

    rtsp.Init(&cbinfo);
    rtsp.Start();
    while (!bExitFlag) {
        usleep(100 * 1000);
        // LOG_DEBUG("sleep ");
    }

    LOG_ERROR("app loop exit");
    rtsp.Stop();
    rtsp.UnInit();
#if (ZC_LIVE_TEST && DZC_LIVE_TEST_THREADSHARED)
    g_ZCLiveTestWriterInstance.UnInit();
#endif
    g_ZCStreamMgrCliInstance.UnInit();
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
