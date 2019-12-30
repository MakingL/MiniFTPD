//
// Created by MLee on 2019/12/22.
//

#include <cstring>
#include "tcp.h"
#include "utility.h"
#include "common.h"


CLTCPServer::CLTCPServer(const char *host, unsigned int port) : m_port(port), m_host(host) {
    if ((m_listen_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        utility::unix_error("Cannot create socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    if (host != nullptr) {
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

namespace tcp {
    void close_fd(int fd) {
        if (close(fd) < 0)
            utility::unix_error("Close error");
    }

    ssize_t send_data(int fd, const void *buf, size_t count) {
        ssize_t rc;

        if ((rc = send(fd, buf, count, 0)) < 0)
            utility::unix_error("Send data error");
        return rc;
    }

    size_t recv_data(int fd, void *buf, size_t count) {
        ssize_t rc;

        if ((rc = recv(fd, buf, count, 0)) < 0)
            utility::unix_error("Recv data error");
        return rc;
    }

    /********************************
    * Client/server helper functions
    ********************************/
    /*
    * open_client_fd - open connection to server at <hostname, port>
    *   and return a socket descriptor ready for reading and writing.
    *   Returns -1 and sets errno on Unix error.
    *   Returns -2 and sets h_errno on DNS (gethostbyname) error.
    */
    int open_client_fd(const char *hostname, int port) {
        int client_fd;
        struct hostent *hp;
        struct sockaddr_in server_addr{0};

        if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return -1; /* check errno for cause of error */

        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;

        if (inet_pton(AF_INET, hostname, &server_addr.sin_addr) <= 0)
            utility::unix_error("inet_pton error");

        server_addr.sin_port = htons(port);
        /* Establish a connection with the server */
        if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
            return -1;
        return client_fd;
    }

    /*
    * open_listen_fd - open and return a listening socket on port
    *     Returns -1 and sets errno on Unix error.
    */
    int open_listen_fd(int port) {
        int listenfd, optval = 1;
        struct sockaddr_in serveraddr{0};

        /* Create a socket descriptor */
        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) /* 居然没有使用包裹函数 */
            return -1;

        /* Eliminates "Address already in use" error from bind. */
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                       (const void *) &optval, sizeof(int)) < 0) /* 设置端口复用 */
            return -1;

        /* Listenfd will be an endpoint for all requests to port
        on any IP address for this host */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons((unsigned short) port);
        if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) /* 绑定 */
            return -1;

        /* Make it a listening socket ready to accept connection requests */
        if (listen(listenfd, tcp::LISTENQ) < 0) /* 监听 */
            return -1;
        return listenfd; /* 返回监听套接字 */
    }

    int get_local_ip(char *ip) {
        char host[48] = {0};
        if (gethostname(host, sizeof(host)) < 0)
            return -1;
        struct hostent *hp;
        if ((hp = gethostbyname(host)) == nullptr)
            return -1;

        strcpy(ip, inet_ntoa(*(struct in_addr *) hp->h_addr));
        return 0;

/*        int fd;
        struct sockaddr_in *addr;
        struct ifreq ifr{0};

        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            return -1;
        }

        strcpy(ifr.ifr_ifrn.ifrn_name, "eth0"); *//* 接口的名称 *//*

        if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) { *//* 通过内核获取接口的信息 *//*
            return -1; *//* ioctl以及socket函数出错都会设置errno *//*
        }

        addr = (sockaddr_in *) &ifr.ifr_ifru.ifru_addr; *//* 获得ip地址 *//*
        strcpy(ip, inet_ntoa(addr->sin_addr));
        close(fd);
        return 0; *//* 成功 */
    }
}
