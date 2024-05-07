// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <sys/epoll.h>

#include "zc_type.h"

#define ZC_EPOLL_EVENT_SIZE_DEF 20  // default epoll event size
#define ZC_EPOLL_TIMEOUT_DEF 10     // default timeout 10ms
namespace zc {
class CEpoll {
 public:
    explicit CEpoll(int maxevents = ZC_EPOLL_EVENT_SIZE_DEF, int timeout = ZC_EPOLL_TIMEOUT_DEF);
    ~CEpoll();
    bool Create();
    void Destroy();
    bool Add(int fd, epoll_event *event);
    bool Mod(int fd, epoll_event *event);
    bool Del(int fd, epoll_event *event);
    int Wait();
    const epoll_event *Events() const;
    // overload [] operator
    const epoll_event &operator[](int index) { return m_backEvents[index]; }

 private:
    int m_epfd;
    int m_timeout;
    int m_maxevents;
    epoll_event *m_backEvents;
};
}  // namespace zc
