// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <errno.h>
#include <unistd.h>

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"

#include "Epoll.hpp"

namespace zc {

CEpoll::CEpoll(int timeout, int maxevents)
    : m_epfd(-1), m_timeout(timeout), m_maxevents(maxevents), m_backEvents(nullptr) {}

CEpoll::~CEpoll() {
    Destroy();
}

void CEpoll::Destroy() {
    if (m_epfd > 0) {
        close(m_epfd);
        m_epfd = -1;
    }
    ZC_SAFE_DELETEA(m_backEvents);
}
#if 0
inline const struct epoll_event *CEpoll::Events() const {
    return m_backEvents;
}
#endif

bool CEpoll::Create() {
    m_epfd = epoll_create(1);
    if (m_epfd == -1) {
        LOG_ERROR("epoll_create error, errno[%d]", errno);
        return false;
    }

    m_backEvents = new struct epoll_event[m_maxevents];
    LOG_TRACE("epoll_create ok m_epfd[%d]", m_epfd);
    return true;
}

bool CEpoll::Add(int fd, unsigned int event, void *ptr) {
    if (m_epfd > 0) {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        ev.data.ptr = ptr;
        if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
            LOG_ERROR("epoll_ctl ADD error m_epfd[%d], fd[%d] ptr[%p], errno[%d]", m_epfd, fd, ev.data.ptr, errno);
            return false;
        }

        LOG_TRACE("Add m_epfd[%d], fd[%d], ptr[%p], event[0x%x]", m_epfd, fd, ptr, event);
        return true;
    }

    return false;
}

bool CEpoll::Mod(int fd, unsigned int event, void *ptr) {
    if (m_epfd > 0) {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        ev.data.ptr = ptr;
        if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev) != 0) {
            LOG_ERROR("epoll_ctl MOD error m_epfd[%d], fd[%d], ptr[%p], errno[%d]", m_epfd, fd, ev.data.ptr, errno);
            return false;
        }
        LOG_TRACE("Mod m_epfd[%d], fd[%d], ptr[%p], event[0x%x]", m_epfd, fd, ptr, event);
        return true;
    }

    return false;
}

bool CEpoll::Del(int fd) {
    if (m_epfd > 0) {
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL) != 0) {
            LOG_ERROR("epoll_ctl DEL error m_epfd[%d], fd[%d] errno[%d]", m_epfd, fd, errno);
            return false;
        }
        return true;
    }

    return false;
}

int CEpoll::Wait() {
    if (m_epfd > 0) {
        return epoll_wait(m_epfd, m_backEvents, m_maxevents, m_timeout);
    }
    return -1;
}
}  // namespace zc
