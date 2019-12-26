//
// Created by MLee on 2019/12/22.
//

#ifndef FTPD_EPOLL_EVENT_H
#define FTPD_EPOLL_EVENT_H

#include "common.h"
#include <sys/epoll.h>
#include <memory>

class CLEpoll {
public:
    CLEpoll(int evt_size = MAX_EVENT);

    void add_event_fd(int fd);

    void wait_for_event();

    epoll_event next_event() {
        return m_events[m_event_iterator++];
    }

    bool has_event() {
        return m_event_iterator >= 0 && m_event_iterator < m_active_event_num;
    }

    static int get_event_fd(epoll_event event) {
        return event.data.fd;
    }

    static bool is_in_event(epoll_event event) {
        return (event.events & EPOLLIN);
    }

private:
    std::unique_ptr<epoll_event[]> m_events;

    int m_epoll_fd;
    int m_active_event_num;
    int m_event_iterator;
    int m_event_size;

    static int set_nonblocking(int fd);
};

#endif //FTPD_EPOLL_EVENT_H
