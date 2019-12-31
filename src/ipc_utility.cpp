//
// Created by MLee on 2019/12/29.
//
#include "common.h"

namespace ipc_utility {
    int send_fd(int sock_fd, int fd) { /* 在进程间传递文件描述符 */
        struct msghdr msg{nullptr};
        struct cmsghdr *p_cmsg;
        struct iovec vec{nullptr};
        char cmsgbuf[CMSG_SPACE(sizeof(fd))];
        int *p_fds;
        char sendchar = 0;
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);
        p_cmsg = CMSG_FIRSTHDR(&msg);
        p_cmsg->cmsg_level = SOL_SOCKET;
        p_cmsg->cmsg_type = SCM_RIGHTS;
        p_cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
        p_fds = (int *) CMSG_DATA(p_cmsg);
        *p_fds = fd;

        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
        msg.msg_iov = &vec;
        msg.msg_iovlen = 1;
        msg.msg_flags = 0;

        vec.iov_base = &sendchar;
        vec.iov_len = sizeof(sendchar);
        return sendmsg(sock_fd, &msg, 0);
    }

    int recv_fd(int sock_fd) { /* 接收文件描述符 */
        int ret;
        struct msghdr msg{nullptr};
        char recvchar;
        struct iovec vec{nullptr};
        int recv_fd;
        char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
        struct cmsghdr *p_cmsg;
        int *p_fd;
        vec.iov_base = &recvchar;
        vec.iov_len = sizeof(recvchar);
        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
        msg.msg_iov = &vec;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);
        msg.msg_flags = 0;

        p_fd = (int *) CMSG_DATA(CMSG_FIRSTHDR(&msg));
        *p_fd = -1;
        ret = recvmsg(sock_fd, &msg, 0);
        if (ret != 1) {
            return -1;
//            utility::unix_error("recvmsg");
        }

        p_cmsg = CMSG_FIRSTHDR(&msg);
        if (p_cmsg == nullptr) {
            return -2;
//            utility::unix_error("no passed fd");
        }


        p_fd = (int *) CMSG_DATA(p_cmsg);
        recv_fd = *p_fd;
        if (recv_fd == -1) {
            return -3;
//            utility::unix_error("no passed fd");
        }

        return recv_fd;
    }
}