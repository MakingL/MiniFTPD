//
// Created by MLee on 2019/12/22.
//

#ifndef FTPD_TCP_H
#define FTPD_TCP_H

#include <string>

class CLTCPServer {
public:
    CLTCPServer(const char *host, unsigned int port);

    CLTCPServer(unsigned int port) {
        CLTCPServer(nullptr, port);
    }

    int start_listen();

    int accept_client(struct sockaddr_in &client_address);

private:
    unsigned int m_port;
    std::string m_host;
    int m_listen_fd;
};

namespace tcp {
    ssize_t send_data(int fd, const void *buf, size_t count);
}

#endif //FTPD_TCP_H
