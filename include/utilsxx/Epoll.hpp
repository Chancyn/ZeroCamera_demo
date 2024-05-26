// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <sys/epoll.h>

#include "zc_type.h"

#define ZC_EPOLL_EVENT_SIZE_DEF 20  // default epoll event size
#define ZC_EPOLL_TIMEOUT_DEF -1     // 10     // default timeout 10ms
namespace zc {
class CEpoll {
 public:
    explicit CEpoll(int timeout = ZC_EPOLL_TIMEOUT_DEF, int maxevents = ZC_EPOLL_EVENT_SIZE_DEF);
    ~CEpoll();
    bool Create();
    void Destroy();
    bool Add(int fd, unsigned int event, void *ptr = nullptr);
    bool Mod(int fd, unsigned int event, void *ptr = nullptr);
    bool Del(int fd);
    int Wait();
    const struct epoll_event *Events() const { return m_backEvents; }
    // overload [] operator
    const struct epoll_event &operator[](int index) { return m_backEvents[index]; }

 private:
    int m_epfd;
    int m_timeout;
    const int m_maxevents;
    struct epoll_event *m_backEvents;
};
}  // namespace zc
