//
// Created by MLee on 2019/12/24.
//

#ifndef MINIFTPD_FTP_HANDLER_H
#define MINIFTPD_FTP_HANDLER_H

#include <memory>
#include "pipe_wrapper.h"

class CLFtpHandler {
public:
    explicit CLFtpHandler(int connect_fd);

    ~CLFtpHandler();

    void start_handle();

private:
    int m_command_fd;
    std::unique_ptr<int []> m_p_pipe_fd;
};


#endif //MINIFTPD_FTP_HANDLER_H
