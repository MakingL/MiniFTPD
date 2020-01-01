//
// Created by MLee on 2019/12/26.
//

#include <random>
#include "data_handle.h"
#include "common.h"
#include "utility.h"
#include "ipc_utility.h"
#include "tcp.h"
#include "configure.h"


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
    int listen_fd = 0;
    ipc_utility::EMState state;
    /* Create a socket descriptor */
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        state = ipc_utility::Error;
        send_ipc_msg(&state, sizeof(state));
        utility::unix_error("Cannot get socket");
    }

    /* Eliminates "Address already in use" error from bind. */
    int reuse = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &reuse, sizeof(int)) < 0) { /* 设置端口复用 */
        state = ipc_utility::Error;
        send_ipc_msg(&state, sizeof(state));
        utility::unix_error("Set socket opt error");
    }

    int port_low = configure::PASV_PORT_LOW;
    int port_high = configure::PASV_PORT_HIGH;
    static std::default_random_engine random_eng(time(nullptr));
    static std::uniform_int_distribution<unsigned int> u_rand(port_low, port_high);
    int first_port_tried = u_rand(random_eng); /* 生成指定范围内的均匀分布的随机数 */

    /* Listenfd will be an endpoint for all requests to port
    on any IP address for this host */
    struct sockaddr_in server_addr{0};
    bzero((char *) &server_addr, sizeof(server_addr));
    int port = first_port_tried;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    while (true) {
        server_addr.sin_port = htons((unsigned short) port);
        if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == 0) {
            break;  /* 绑定成功 */
        }

        --port;
        if (port < port_low) {
            port = port_high;
        }
        if (port == first_port_tried) {
            close(listen_fd);
            /* 告知父进程 */
            state = ipc_utility::PortExhausted;
            send_ipc_msg(&state, sizeof(state));
            return;
        }
    }
    /* Make it a listening socket ready to accept connection requests */
    if (listen(listen_fd, tcp::LISTENQ) < 0) { /* 监听 */
        close(listen_fd);
        state = ipc_utility::Error;
        send_ipc_msg(&state, sizeof(state));
        utility::unix_error("Listen error");
        return;
    }

    m_data_fd = listen_fd;
    m_port = port;
    get_local_ip(); /* 获得本地 IP 地址 */

    state = ipc_utility::Success;
    send_ipc_msg(&state, sizeof(state));

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
        } else {
            tcp::close_fd(m_data_fd);   /* Client 已连接，停止监听该端口，以及时释放该端口资源 */
        }
    } else {    /* PORT 模式，主动连接 client */
        if ((fd = tcp::open_client_fd(m_ip_addr.c_str(), m_port, configure::PORT_CONN_PORT)) < 0) {
            utility::debug_info("Cannot connect to PORT client");
            return;
        }
        utility::debug_info(std::string("PORT local port: ") + std::to_string(configure::PORT_CONN_PORT));
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
