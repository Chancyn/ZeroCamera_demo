// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "zc_type.h"

#include "ZCLog.hpp"

#include "zc_log.h"

#if ZC_DEBUG
#include <string>
#endif

#if ZC_DEBUG
#define ZC_LOG_CONS_DEF_LEVEL LEVEL_TRACE
#define ZC_LOG_DEF_LEVEL LEVEL_DEBUG
#else
#define ZC_LOG_CONS_DEF_LEVEL LEVEL_WARN
#define ZC_LOG_DEF_LEVEL LEVEL_DEBUG
#endif

#define ZC_LOG_MAX_SIZE (1024 * 1024 * 10)  // file max size
#define ZC_LOG_MAX_NUM (2)                  // file max num

#define MSG_BUF_LEN 1024
static bool g_binit = false;

void zc_log_levelout(int level, const char *fmt, ...) {
    char buf[MSG_BUF_LEN] = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, MSG_BUF_LEN - 1, fmt, args);
    va_end(args);
    if (likely(g_binit)) {
        logxx_levelout(static_cast<spdlog::level::level_enum>(level), buf);
    } else {
        printf("uninitlog [%s] %s\n",
               spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(level)).data(), buf);
    }

    return;
}

// for test
static void zc_log_test() {
#if ZC_DEBUG
    std::string teststr = {"bbb std::string"};
    int num = 3;
    float fnum = 3.1415;
    char cnum = 'A';
    const char *str = "aaa";

    // c style outlog printf
    LOG_TRACE("clog testtrace stdstring %s", teststr.c_str());
    LOG_DEBUG("clog testdebug cstring %s", str);
    LOG_INFO("clog testinfo num %d", num);
    LOG_WARN("clog testwarn char %c", cnum);
    LOG_ERROR("clog testerror float %.2f", fnum);
    LOG_CRITI("clog testcriti cstring+num %s %d %s", str, num, teststr.c_str());

    // cxx style outlog
    logxx_trace("cxx testtrace stdstring {}", teststr);
    logxx_debug("cxx testdebug cstring {}", str);
    logxx_info("cxx testinfo num {}", num);
    logxx_warn("cxx testwarn char {}", cnum);
    logxx_error("cxx testerror float {:.02f}", fnum);
    logxx_criti("cxx testcriti cstring+num {} {} {}", str, num, teststr);
#endif
    return;
}

int zc_log_init(const char *szName) {
    zc_logconf_t conf = {.level = ZC_LOG_DEF_LEVEL,
                         .fulshlevel = LEVEL_TRACE,
                         .conslevel = ZC_LOG_CONS_DEF_LEVEL,
                         .size = ZC_LOG_MAX_SIZE,
                         .count = ZC_LOG_MAX_NUM,
                         "./app.log"};
    if (!g_binit) {
        g_ZCLogInstance.Init(conf);
        // zc_log_test();
        g_binit = true;
    }

    LOG_INFO("init ok");
    return 0;
}

int zc_log_uninit() {
    if (g_binit) {
        g_binit = false;
        g_ZCLogInstance.UnInit();
    }

    return 0;
}
