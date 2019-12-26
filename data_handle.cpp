//
// Created by MLee on 2019/12/26.
//

#include "data_handle.h"
#include "common.h"
#include "utility.h"


CLDataHandle::CLDataHandle(int pipe_fd)
        : m_b_stop(false),
          m_pipe_fd(pipe_fd),
          m_data_fd(0) {

}

void CLDataHandle::handle() {
    char cmd[1024] = {0};
    while (!m_b_stop) {
        /* TODO： 不断从命令进程获取命令，并执行后从数据连接返回结果 */
        /* TODO: epoll 端口复用 */
        int ret = recv(m_pipe_fd, cmd, sizeof(cmd), 0);
        if (ret == -1) {
            utility::unix_error("Receive command error");
        }
    }
}

CLDataHandle::~CLDataHandle() {
    utility::debug_info("Close data link");
    if (m_data_fd) {
        close(m_data_fd);
    }
    close(m_pipe_fd);
}


