//
// Created by MLee on 2019/12/23.
//

#include "pipe_wrapper.h"
#include "utility.h"

CLPipeWrapper::CLPipeWrapper(int *pipe_fd) : m_p_pipe(pipe_fd) {
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, pipe_fd) == -1) {
        utility::unix_error("Can't create IPC pipe");
    }
}

void CLPipeWrapper::set_write_nonblock() {
    int old_option = fcntl(m_p_pipe[1], F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(m_p_pipe[1], F_SETFL, new_option);
}

