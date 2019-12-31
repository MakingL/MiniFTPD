//
// Created by MLee on 2019/12/22.
//

#include "epoll_event.h"
#include "utility.h"
#include <sys/epoll.h>

CLEpoll::CLEpoll(int evt_size) : m_event_size(evt_size) {
    if ((m_epoll_fd = epoll_create(m_event_size)) < 0) {
        utility::unix_error("Create event epoll error");
    }

    m_event_iterator = -1;
    m_active_event_num = 0;
    auto p_epoll_event = new epoll_event[m_event_size];
    m_events = std::unique_ptr<epoll_event[]>(p_epoll_event);
}

void CLEpoll::add_event_fd(int fd) {
    epoll_event event{0};
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event);

    set_nonblocking(fd);
}

int CLEpoll::set_nonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void CLEpoll::wait_for_event() {
    m_active_event_num = epoll_wait(m_epoll_fd, m_events.get(), m_event_size, -1);
    if ((m_active_event_num < 0) && (errno != EINTR)) {
        utility::unix_error("CLEpoll wait failure");
    }
    m_event_iterator = 0;

//    utility::debug_info("Waited a epoll event: " + std::to_string(m_active_event_num));
}
