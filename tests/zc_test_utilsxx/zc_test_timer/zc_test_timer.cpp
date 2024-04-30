// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "zc_list.h"
#include "zc_log.h"

#include "ZCTimer.hpp"
#include "zc_test_timer.hpp"

void test_timer_cb_once(void *pdata, size_t len, void *pcontext) {
    LOG_INFO("test_timer_cb into pdata[%p], len[%d], pcontext[%p]", pdata, len, pcontext);

    return;
}

void test_timer_cb_cnt(void *pdata, size_t len, void *pcontext) {
    static int nCnt = 0;
    nCnt++;
    LOG_DEBUG("timer_cb_cnt into nCnt[%d] pdata[%p], len[%d], pcontext[%p]", nCnt, pdata, len, pcontext);

    return;
}

void test_timer_cb_cnt_stop(void *pdata, size_t len, void *pcontext) {
    static int nCnt = 0;
    nCnt++;
    LOG_WARN("test_timer_cb_cnt_stop into nCnt[%d] pdata[%p], len[%d], pcontext[%p]", nCnt, pdata, len, pcontext);

    return;
}
void test_timer_cb_forever(void *pdata, size_t len, void *pcontext) {
    static int nCnt = 0;
    nCnt++;
    LOG_ERROR("test_timer_cb_forever into nCnt[%d] pdata[%p], len[%d], pcontext[%p]", nCnt, pdata, len, pcontext);

    return;
}
#define TEST_TIMER_EXOIRES 10

static int test_timer_once() {
    LOG_INFO("test into\n");
    {
        int waitcnt = 0;
        zc::CTimer timerA;
        struct zc_timer_info infoA = {};
        infoA.expires = TEST_TIMER_EXOIRES;
        infoA.timercnt = 1;
        infoA.period = TEST_TIMER_EXOIRES;
        infoA.fun_cb = test_timer_cb_once;
        infoA.pdata = (void *)"test_timer_cb_once";
        infoA.len = strlen((const char *)infoA.pdata);
        infoA.pcontext = (void *)test_timer_cb_once;
        timerA.Start(&infoA);

        while (waitcnt++ < 5) {
            sleep(1);
        }
    }

    LOG_INFO("test exit\n");

    return 0;
}

static int test_timer_timecnt() {
    LOG_INFO("test into\n");
    int waitcnt = 0;
    {
        zc::CTimer timerA;
        struct zc_timer_info infoA = {};
        infoA.expires = TEST_TIMER_EXOIRES;
        infoA.timercnt = 5;
        infoA.period = TEST_TIMER_EXOIRES;
        infoA.fun_cb = test_timer_cb_cnt;
        infoA.pdata = (void *)"test_timer_cb_cnt";
        infoA.len = strlen((const char *)infoA.pdata) + 1;
        infoA.pcontext = (void *)test_timer_cb_cnt;
        timerA.Start(&infoA);

        while (waitcnt++ < 10) {
            sleep(1);
        }
    }

    LOG_INFO("test exit\n");

    return 0;
}

static int test_timer_timecnt_stop() {
    LOG_INFO("test into\n");
    int waitcnt = 0;
    {
        zc::CTimer timerA;
        struct zc_timer_info infoA = {};
        infoA.expires = TEST_TIMER_EXOIRES;  // TEST_TIMER_EXOIRES;
        infoA.timercnt = 5;
        infoA.period = TEST_TIMER_EXOIRES;  // TEST_TIMER_EXOIRES;
        infoA.fun_cb = test_timer_cb_cnt_stop;
        infoA.pdata = (void *)"test_timer_cb_cnt_stop";
        infoA.len = strlen((const char *)infoA.pdata) + 1;
        infoA.pcontext = (void *)test_timer_cb_cnt_stop;
        timerA.Start(&infoA);

        while (waitcnt++ < 10) {
            if (waitcnt == 6) {
                LOG_INFO("test manual stop");
                timerA.Stop();
            }
            sleep(1);
        }
    }

    LOG_INFO("test exit\n");

    return 0;
}

static int test_timer_forever() {
    LOG_INFO("test into\n");
    int waitcnt = 0;
    {
        zc::CTimer timerA = {};
        struct zc_timer_info infoA;
        infoA.expires = TEST_TIMER_EXOIRES;
        infoA.timercnt = -1;
        infoA.period = TEST_TIMER_EXOIRES;
        infoA.fun_cb = test_timer_cb_forever;
        infoA.pdata = (void *)"test_timer_cb_forever";
        infoA.len = strlen((const char *)infoA.pdata) + 1;
        infoA.pcontext = (void *)test_timer_cb_forever;
        timerA.Start(&infoA);

        while (waitcnt++ < 20) {
            sleep(1);
        }
    }

    LOG_INFO("test exit\n");

    return 0;
}

static int test_timer() {
    g_ZCTimerManagerInstance.Init();
    test_timer_once();
    test_timer_timecnt();
    test_timer_timecnt_stop();
    // test_timer_forever();
    g_ZCTimerManagerInstance.UnInit();

    return 0;
}

int zc_test_timer() {
    LOG_INFO("test into\n");
    test_timer();
    LOG_INFO("test exit\n");

    return 0;
}
