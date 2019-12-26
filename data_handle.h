//
// Created by MLee on 2019/12/26.
//

#ifndef MINIFTPD_DATA_HANDLE_H
#define MINIFTPD_DATA_HANDLE_H

class CLDataHandle {
public:
    explicit CLDataHandle(int write_pipe_fd);

    ~CLDataHandle();

    void handle();

private:
    int m_pipe_fd;
    int m_data_fd;

    bool m_b_stop;
};

#endif //MINIFTPD_DATA_HANDLE_H
