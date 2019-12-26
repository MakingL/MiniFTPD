//
// Created by MLee on 2019/12/22.
//

#include "signal_wrapper.h"
#include "common.h"
#include "utility.h"

CLSignalWrapper::CLSignalWrapper() {
}

void CLSignalWrapper::add_signal(int sig, void(*handler)(int), bool restart) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart) {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    if (sigaction(sig, &sa, nullptr) == -1) {
        utility::unix_error("Add signal error");
    }
}

