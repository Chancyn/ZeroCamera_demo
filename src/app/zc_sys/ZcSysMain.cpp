// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zc_log.h"
#include "zc_proc.h"

#include "ZcStreamMgr.hpp"
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

// streamMgr handle mod msg callback
static int StreamMgrHandleMsg(void *ptr, unsigned int type, void *indata, void *outdata) {
    // LOG_TRAE("StreamMgrCb ptr:%p, type:%d, indata:%d", ptr, type);
    return  g_ZCStreamMgrInstance.HandleCtrl(type, indata, outdata);
}

// SysManager handle mod msg callback
static int SysMgrHandleMsg(void *ptr, unsigned int type, void *indata, void *outdata) {
    // LOG_TRACE("SysMgrCb ptr:%p, type:%d", ptr, type);
    if (type >= SYS_SMGR_HDL_BUTT_E)
        return -1;

    return -1;
}

int main(int argc, char **argv) {
    printf("main into\n");
    // ZC_PROC_SETNAME(ZC_APP_NAME);
    char pname[ZC_MAX_PNAME] = {0};
    ZC_PROC_GETNAME(pname, ZC_MAX_PNAME);

    InitSignals();
    zc_log_init(ZC_LOG_PATH ZC_LOG_APP_NAME);
    LOG_INFO("process[%s,%s], build:[%s]\n", argv[0], pname, ZcGetBuildDateTimeStr());

    zc::smgr_callback_info_t smgrcbinfo = {
        .publishMsgCb = zc::CSysManager::PublishMsgCb,
        .Context = &g_ZCSysManagerInstance,
    };
    g_ZCStreamMgrInstance.Init(NULL, &smgrcbinfo);

    zc::sys_callback_info_t cbinfo = {
        .streamMgrHandleCb = StreamMgrHandleMsg,
        .streamMgrContext = nullptr,
        .MgrHandleCb = SysMgrHandleMsg,
        .MgrContext = &g_ZCSysManagerInstance,
    };

    g_ZCSysManagerInstance.Init(&cbinfo);
    g_ZCStreamMgrInstance.Start();
    g_ZCSysManagerInstance.Start();
    while (!bExitFlag) {
        usleep(100 * 1000);
        // LOG_TRACE("sleep ");
    }

    LOG_ERROR("app loop exit");
    g_ZCSysManagerInstance.Stop();
    g_ZCSysManagerInstance.UnInit();
    g_ZCStreamMgrInstance.Stop();
    g_ZCStreamMgrInstance.UnInit();
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
