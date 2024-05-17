// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zc_log.h"

#include "zc_test_fifo_lock.hpp"
#include "zc_test_fifo_nolock.hpp"
#include "zc_test_shmfifo.hpp"
#include "zc_test_shmfifoev.hpp"
#include "zc_test_mod.hpp"
#include "zc_test_msgcomm.hpp"
#include "zc_test_utilsxx.hpp"

#define ZC_LOG_PATH "./log"
#define ZC_LOG_APP_NAME "zc_app.log"

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

    int nodetype = 0;
    int fifotype = 0;
    if (argc > 2) {
        fifotype = atoi(argv[1]);
        nodetype = atoi(argv[2]);
    }

    LOG_DEBUG("NODE type [%d]", nodetype);

    // zc_test_utilsxx();
    // zc_test_mod();
    // zc_test_msgcomm_start(nodetype);
    // zc_test_mod_start(nodetype);

    // lock-nolock test
    // if (fifotype == 0) {
    //     zc_test_fifo_nolock_start(nodetype);
    // } else {
    //     zc_test_fifo_lock_start(nodetype);
    // }

    // zc_test_utilsxx_epoll_start();
    // shm fifo test
     zc_test_shmfifoev_start(fifotype, nodetype);

    // zc_test_utilsxx_semaphore();
    // zc_test_utilsxx_unsemaphore();
    while (!bExitFlag) {
        sleep(1);
        LOG_DEBUG("sleep exit");
    }
    LOG_ERROR("app loop exit");
    // zc_test_utilsxx_epoll_stop();
     zc_test_shmfifoev_stop(nodetype);
    // if (fifotype == 0) {
    //     zc_test_fifo_nolock_stop(nodetype);
    // } else {
    //     zc_test_fifo_lock_stop(nodetype);
    // }

    // zc_test_mod_stop(nodetype);
    //  zc_test_msgcomm_stop();
    zc_log_uninit();
    printf("main exit\n");
    return 0;
}
/*spinlock版本fifo
### 无锁队列测试性能 1000000 次，每次写入1024个字节耗时105ms速度 1024000000/105ms=2459M/s
[2024-05-05 19:53:06.888][info][tid 226967] [zc_test_fifo_lock.cpp 52][process]ThreadPutLock cnt[1000000]errcnt[5234015],ret[1024000000],cos[397]ms
[2024-05-05 19:53:06.888][info][tid 226968] [zc_test_fifo_lock.cpp 96][process]ThreadGetLock cnt[1000000]errcnt[166652],ret[1024000000],cos[397]ms
*/