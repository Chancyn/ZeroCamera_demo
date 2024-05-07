// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "Epoll.hpp"
#include "Thread.hpp"
#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_test_epoll.hpp"

class CTestEpollTh : public zc::Thread {
 public:
    explicit CTestEpollTh(std::string name) : Thread(name), m_name(name), m_process_cnt(0) {
        LOG_WARN("Constructor into [%s]\n", m_name.c_str());
    }
    virtual ~CTestEpollTh() { LOG_WARN("~Destructor into [%s]\n", m_name.c_str()); }
    virtual int process() {
        m_process_cnt++;
        int ret = 0;
        zc::CEpoll ep;
        if (!ep.Create()) {
            LOG_ERROR("epoll create error");
            return -1;
        }

        while (State() == Running) {
            ret = ep.Wait();
            if (ret == -1) {
                LOG_ERROR("epoll wait error");
                return -1;
            }
        }
        return 0;
    }

 private:
    std::string m_name;
    unsigned int m_process_cnt;
};

static CTestEpollTh *g_pEpollth = nullptr;
static int test_epoll() {
    LOG_INFO("test into\n");
    g_pEpollth = new CTestEpollTh("epoll_test");
    ZC_ASSERT(g_pEpollth != nullptr);
    g_pEpollth->Start();

    return 0;
}

int zc_test_epoll_start() {
    LOG_INFO("test into\n");

    test_epoll();
    LOG_INFO("test exit\n");

    return 0;
}

int zc_test_epoll_stop() {
    LOG_INFO("test into\n");
    ZC_SAFE_DELETE(g_pEpollth);
    LOG_INFO("test exit\n");

    return 0;
}