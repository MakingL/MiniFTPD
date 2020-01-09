//
// Created by MLee on 2019/12/22.
//

#ifndef FTPD_TCP_H
#define FTPD_TCP_H

#include <string>
#include <set>
#include <unordered_map>
#include "common.h"
#include "utility.h"

class CLTCPServer {
public:
    CLTCPServer(const char *host, unsigned int port);

    explicit CLTCPServer(unsigned int port) : CLTCPServer(nullptr, port) {

    }


    int start_listen();

    int accept_client(struct sockaddr_in &client_address);

    bool limit_client_crowding(int connect_fd, unsigned int client_ip); /* 限流*/
    void on_a_client_exit(unsigned int client_ip);

private:
    unsigned int m_port;
    std::string m_host;
    int m_listen_fd;

    /* 限流 */
    std::unordered_map<unsigned int, unsigned int> m_client_ip_mapper;  /* client ip 到 连接数量的映射 */
    std::set<unsigned int> m_client_counter;  /* 当前建立连接的 client 数量 */
};

namespace tcp {
    void close_fd(int fd);

    class CLConnection {
    public:
        explicit CLConnection(int fd) : m_sock_fd(fd) {

        }

        ~CLConnection() {
            tcp::close_fd(m_sock_fd);
        }

        int get_fd() {
            return m_sock_fd;
        }

    private:
        int m_sock_fd;
    };
}

namespace tcp {
    ssize_t send_data(int fd, const void *buf, size_t count);

    size_t recv_data(int fd, void *buf, size_t count);
    /********************************
    * Client/server helper functions
    ********************************/
    /*
    * open_client_fd - open connection to server at <hostname, port>
    *   and return a socket descriptor ready for reading and writing.
    *   Returns -1 and sets errno on Unix error.
    *   Returns -2 and sets h_errno on DNS (gethostbyname) error.
    */
    /* $begin open_client_fd */
    int open_client_fd(const char *hostname, int port, int client_port=0);
    /* $end open_client_fd */

    /*
    * open_listen_fd - open and return a listening socket on port
    *     Returns -1 and sets errno on Unix error.
    */
    /* $begin open_listen_fd */
    const int LISTENQ = 1024;   /* second argument to listen() */
    int open_listen_fd(int port);

    /* 获取本机公网 IP 地址 */
    int get_local_ip(char *ip);
}

#endif //FTPD_TCP_H
