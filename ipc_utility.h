//
// Created by MLee on 2019/12/29.
//

#ifndef MINIFTPD_IPC_UTILITY_H
#define MINIFTPD_IPC_UTILITY_H

namespace ipc_utility {
    enum EMIpcCmd {     /* FTP command 进程和 data 进程间的命令 */
        k_ExecPASV = 0,    /* 获得 PASV 模式下的端口号 */
        k_GetFd,  /* 获取 建立的连接 */
        k_PeerInfo,  /* PORT 模式下的 client 地址信息 */
        k_Exit, /* 退出 */
    };

    enum EMState {  /* 操作状态 */
        Error,
        Success,
        ReadError,
        WriteError,
    };

    int send_fd(int sock_fd, int fd);   /* 在进程间传递文件描述符 */
    int recv_fd(int sock_fd);   /* 接收文件描述符 */
}

#endif //MINIFTPD_IPC_UTILITY_H
