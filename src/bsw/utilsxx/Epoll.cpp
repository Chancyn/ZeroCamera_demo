// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <errno.h>
#include <unistd.h>

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"

#include "Epoll.hpp"

namespace zc {

CEpoll::CEpoll(int maxevents, int timeout)
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

inline const epoll_event *CEpoll::Events() const {
    return m_backEvents;
}

bool CEpoll::Create() {
    m_epfd = epoll_create(1);
    if (m_epfd == -1) {
        LOG_ERROR("epoll_create error, errno[%d]", errno);
        return false;
    }

    m_backEvents = new epoll_event[m_maxevents];
    LOG_TRACE("epoll_create ok m_epfd[%d]", m_epfd);
    return true;
}

bool CEpoll::Add(int fd, unsigned int event) {
    if (m_epfd > 0) {
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = event;
        if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
            LOG_ERROR("epoll_ctl ADD error m_epfd[%d], fd[%d] errno[%d]", m_epfd, fd, errno);
            return false;
        }

        LOG_TRACE("Add m_epfd[%d], fd[%d], event[0x%x]", m_epfd, fd, event);
        return true;
    }

    return false;
}

bool CEpoll::Mod(int fd, unsigned int event) {
    if (m_epfd > 0) {
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = event;
        if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev) != 0) {
            LOG_ERROR("epoll_ctl MOD error m_epfd[%d], fd[%d] errno[%d]", m_epfd, fd, errno);
            return false;
        }
        LOG_TRACE("Mod m_epfd[%d], fd[%d], event[0x%x]", m_epfd, fd, event);
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
