//
// Created by MLee on 2019/12/26.
//

#include "data_handle.h"
#include "common.h"
#include "utility.h"
#include "ipc_utility.h"
#include "tcp.h"


CLDataHandle::CLDataHandle(int pipe_fd)
        : m_b_stop(false),
          m_b_pasv_mode(true),
          m_port(0),
          m_pipe_fd(pipe_fd),
          m_data_fd(-1) {

}

void CLDataHandle::handle() {
    ipc_utility::EMIpcCmd cmd;
    while (!m_b_stop) {
        /* 不断从命令进程获取命令，并执行后返回结果给父进程 */
        /* TODO: 考虑 epoll I/O 复用 */
        int ret = recv_ipc_msg(&cmd, sizeof(cmd));
        if (ret == -1) {
            utility::unix_error("Receive command error");
        } else if (ret == 0) {
            /* IPC 管道已经关闭 */
            m_b_stop = true;
        }

        switch (cmd) {
            case ipc_utility::k_ExecPASV:
                utility::debug_info("Data subprocess receive k_ExecPASV cmd");
                m_b_pasv_mode = true;
                do_PASV();
                break;
            case ipc_utility::k_GetFd:
                utility::debug_info("Data subprocess receive k_GetFd cmd");
                do_get_fd();
                break;
            case ipc_utility::k_PeerInfo:
                utility::debug_info("Data subprocess receive k_PeerInfo cmd");
                m_b_pasv_mode = false;
                do_peer_info();
                break;
            case ipc_utility::k_Exit:
                utility::debug_info("Data subprocess receive k_Exit cmd");
                m_b_stop = true;
                break;
            default:
                break;
        }
    }

    utility::debug_info("Data link process exit");
    exit(EXIT_SUCCESS);
}

CLDataHandle::~CLDataHandle() {
    close(m_pipe_fd);
}

void CLDataHandle::do_PASV() {
    /* 随机监听一个数据端口 */
    m_data_fd = tcp::open_listen_fd(0); /* 0代表任意选择一个端口 */
    if (m_data_fd < 0) {
        utility::unix_error("Cannot listen a data fd");
    }
    struct sockaddr_in local_addr{0};
    socklen_t len = sizeof(local_addr);
    if (getsockname(m_data_fd, (struct sockaddr *) &local_addr, &len) < 0) {
        utility::unix_error("Get sock name error");
    }
    m_port = ntohs(local_addr.sin_port); /* 得到端口号 */

    get_local_ip(); /* 获得本地 IP 地址 */

    int ip_size = m_ip_addr.size();
    send_ipc_msg(&ip_size, sizeof(ip_size));   /* 先传递长度 */
    send_ipc_msg(m_ip_addr.c_str(), ip_size);  /* 传递ip地址 */
    send_ipc_msg(&m_port, sizeof(m_port)); /* 传递端口值 */
}

void CLDataHandle::do_peer_info() {
    /* 接收对端节点的地址信息 */
    char ip_addr[20] = {0};
    recv_ipc_msg(&m_port, sizeof(m_port)); /* 获得端口 */
    recv_ipc_msg(ip_addr, sizeof(ip_addr)); /* 获得ip地址 */
    m_ip_addr.assign(ip_addr, ip_addr + strlen(ip_addr));
}

void CLDataHandle::do_get_fd() {
    int fd = 0;
    if (m_b_pasv_mode) {    /* PASV 模式，等待 client 的连接 */
        if ((fd = accept(m_data_fd, nullptr, nullptr)) < 0) {
            utility::debug_info("Accept client data link error");
            return;
        }
    } else {    /* PORT 模式，主动连接 client */
        if ((fd = tcp::open_client_fd(m_ip_addr.c_str(), m_port)) < 0) {
            utility::debug_info("Cannot connect to PORT client");
            return;
        }
    }

    ipc_utility::EMState state = ipc_utility::Success;
    send_ipc_msg(&state, sizeof(state));   /* 发送成功的状态 */

    if (ipc_utility::send_fd(m_pipe_fd, fd) < 0) {  /* 将建立起的数据连接发送给父进程 */
        utility::unix_error("Send fd error");
    }

    close(fd);  /* 本进程关闭数据连接，数据连接由父进程去管理 */
}

const char *CLDataHandle::get_local_ip() {
    if (!m_ip_addr.empty() && m_b_pasv_mode) return m_ip_addr.c_str();

    char ip[48] = {0};
    if (tcp::get_local_ip(ip) < 0) {
        utility::unix_error("Cannot get local ip");
    }
    m_ip_addr = ip;
    return m_ip_addr.c_str();
}
