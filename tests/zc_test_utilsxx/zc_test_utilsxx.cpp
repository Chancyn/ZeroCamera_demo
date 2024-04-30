// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "zc_log.h"
#include "ZCTimer.hpp"

#include "zc_test_thread/zc_test_thread.hpp"
#include "zc_test_observer/zc_test_observer.hpp"
#include "zc_test_singleton/zc_test_singleton.hpp"
#include "zc_test_timer/zc_test_timer.hpp"
#include "zc_test_utilsxx.hpp"

int zc_test_utilsxx() {
    LOG_INFO("test_utilsxx into\n");
    zc_test_Instance();
    zc_test_observer();
    zc_test_thread();
    zc_test_timer();
    LOG_INFO("test_utilsxx exit\n");

    return 0;
}
