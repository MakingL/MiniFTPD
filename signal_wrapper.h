//
// Created by MLee on 2019/12/22.
//

#ifndef MINIFTPD_SIGNAL_WRAPPER_H
#define MINIFTPD_SIGNAL_WRAPPER_H

#include "epoll_event.h"
#include "pipe_wrapper.h"

class CLSignalWrapper {
public:
    CLSignalWrapper();

    void add_signal(int sig, void(*handler)(int), bool restart = true);

    void ignore_signal(int sig) {
        add_signal(sig, SIG_IGN);
    }
};



#endif //MINIFTPD_SIGNAL_WRAPPER_H
