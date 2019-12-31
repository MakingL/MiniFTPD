//
// Created by MLee on 2019/12/23.
//

#ifndef MINIFTPD_PIPE_WRAPPER_H
#define MINIFTPD_PIPE_WRAPPER_H

#include "common.h"


class CLPipeWrapper {
public:
    explicit CLPipeWrapper(int pipe_fd[2]);

    void set_write_nonblock();

    int get_read_fd() {
        return m_p_pipe[0];
    }

    int get_write_fd() {
        return m_p_pipe[1];
    }

    inline int get_a_signal(char *signals, int buff_len) {
        return recv(m_p_pipe[0], signals, buff_len, 0);
    }

private:
    int *m_p_pipe;
};


#endif //MINIFTPD_PIPE_WRAPPER_H
