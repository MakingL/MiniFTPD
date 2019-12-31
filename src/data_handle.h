//
// Created by MLee on 2019/12/26.
//

#ifndef MINIFTPD_DATA_HANDLE_H
#define MINIFTPD_DATA_HANDLE_H

#include <string>
#include "common.h"

class CLDataHandle {
public:
    explicit CLDataHandle(int write_pipe_fd);

    ~CLDataHandle();

    void handle();

private:
    void do_PASV();
    void do_peer_info();
    void do_get_fd();

    const char *get_local_ip();
private:
    size_t send_ipc_msg(const void *msg, int len) { /* 发送 ipc 信息给子进程 */
        return send(m_pipe_fd, msg, len, 0);
    }

    size_t recv_ipc_msg(void *msg, int len) { /* 接收来自子进程的 ipc 信息 */
        return recv(m_pipe_fd, msg, len, 0);
    }
private:
    int m_pipe_fd;
    int m_data_fd;

    bool m_b_stop;
    bool m_b_pasv_mode;
    std::string m_ip_addr;
    uint16_t m_port;
};

#endif //MINIFTPD_DATA_HANDLE_H
