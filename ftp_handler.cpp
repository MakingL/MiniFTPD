//
// Created by MLee on 2019/12/24.
//

#include "ftp_handler.h"
#include "common.h"
#include "utility.h"
#include "command_handle.h"
#include "data_handle.h"

CLFtpHandler::CLFtpHandler(int connect_fd) :
        m_command_fd(connect_fd),
        m_p_pipe_fd(std::unique_ptr<int[]>(new int[2])) {
}

void CLFtpHandler::start_handle() {
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, m_p_pipe_fd.get()) == -1) {
        utility::unix_error("Can't create IPC pipe");
    }

    pid_t pid = fork();
    if (pid == -1) {
        utility::unix_error("Create new process error");
    }

    if (pid == 0) {
        /* 父进程退出，给子进程发送 SIGKILL 信号 */
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        close(m_command_fd);
        /* 关闭管道读的一端 */
        close(m_p_pipe_fd[0]);

        /* 数据连接处理 */
        CLDataHandle dataHandler(m_p_pipe_fd[1]);
        dataHandler.handle();

    } else { /* 父进程 */
        utility::debug_info(std::string("Forked process: ") + std::to_string(pid));
        /* 关闭管道写的一端 */
        close(m_p_pipe_fd[1]);

        /* 处理客户端的命令 */
        CLCommandHandle cmd_handler(m_command_fd, m_p_pipe_fd[0]);
        cmd_handler.handle();
    }
}

CLFtpHandler::~CLFtpHandler() = default;
