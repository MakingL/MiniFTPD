//
// Created by MLee on 2019/12/22.
//

#ifndef FTPD_UTILITY_H
#define FTPD_UTILITY_H

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include "common.h"

namespace utility {
    inline void unix_error(const char *msg) {
        perror(msg);
        exit(EXIT_FAILURE);
    }

    inline void debug_info(const std::string &info) {
#ifndef NDEBUG
        std::cout << info << std::endl;
#endif
    }

    inline void debug_socket_info(int fd, const std::string &info) {
#ifndef NDEBUG
        std::cout << info << std::endl;
        std::string data = info + "\n";
        send(fd, data.c_str(), data.length(), 0);
#endif
    }

}

#endif //FTPD_UTILITY_H
