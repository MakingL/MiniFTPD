//
// Created by MLee on 2019/12/22.
//

#include <cstring>
#include "tcp.h"
#include "utility.h"
#include "common.h"


CLTCPServer::CLTCPServer(const char *host, unsigned int port) {
    if ((m_listen_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        utility::unix_error("Cannot create socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    m_port = port;
    if (host != nullptr) {
        m_host = host;

        if (inet_pton(AF_INET, host, &server_addr.sin_addr) == 0) {
            /* 尝试通过域名解析地址 */
            struct hostent *host_info = gethostbyname(host);
            if (host_info == nullptr) {
                utility::unix_error("Get host by name error");
            } else {
                server_addr.sin_addr = *(struct in_addr *) host_info->h_addr;
            }
        }
    } else {
        /* 任意主机 */
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    server_addr.sin_port = htons(port);

    /* 设置端口重用 */
    int port_reuse = 1;
    if (setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, &port_reuse, sizeof(port_reuse)) < 0) {
        utility::unix_error("Set port reuse error");
    }

    if (bind(m_listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        utility::unix_error("Bind server socket error");
    }
}

int CLTCPServer::start_listen() {
    /* 监听， 监听队列长度 SOMAXCONN： 128 */
    if (listen(m_listen_fd, SOMAXCONN) < 0) {
        utility::unix_error("Listen socket error");
    }

    return m_listen_fd;
}

int CLTCPServer::accept_client(struct sockaddr_in &client_address) {
    socklen_t client_addr_len = sizeof(client_address);
    return accept(m_listen_fd, (struct sockaddr *) &client_address, &client_addr_len);
}


ssize_t tcp::send_data(int fd, const void *buf, size_t count) {
    ssize_t rc;

    if ((rc = send(fd, buf, count, 0)) < 0)
        utility::unix_error("Send data error");
    return rc;
}
