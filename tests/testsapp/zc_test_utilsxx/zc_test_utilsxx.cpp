// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "Timer.hpp"
#include "zc_log.h"

#include "zc_test_semaphore/zc_test_semaphore.hpp"
#include "zc_test_semaphore/zc_test_unsemaphore.hpp"
#include "zc_test_observer/zc_test_observer.hpp"
#include "zc_test_singleton/zc_test_singleton.hpp"
#include "zc_test_thread/zc_test_thread.hpp"
#include "zc_test_epoll/zc_test_epoll.hpp"
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

int zc_test_utilsxx_epoll_start() {
    return zc_test_epoll_start();
}

int zc_test_utilsxx_epoll_stop() {
    return zc_test_epoll_stop();
}

int zc_test_utilsxx_semaphore() {
    return zc_test_semaphore();
}

int zc_test_utilsxx_unsemaphore() {
    return zc_test_unsemaphore();
}

