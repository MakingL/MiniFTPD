#include "common.h"
#include "tcp.h"
#include "epoll_event.h"
#include "utility.h"
#include "pipe_wrapper.h"
#include "signal_wrapper.h"
#include "ftp_handler.h"
#include "configure.h"
#include "ftp_codes.h"


int signal_pipe_fd[2];

void signal_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(signal_pipe_fd[1], (char *) &msg, 1, 0);
    utility::debug_info(std::string("Trigger a signal: " + std::to_string(sig)));
    errno = save_errno;
}

int main(int argc, char *argv[]) {
    utility::debug_info("MiniFTPD Start");

    /* 必须用 root 用户启动，因为其中的用户验证部分需要特权用户才能使用 */
    if (getuid() != 0) {
        std::cerr << "MiniFTPD: must be started as root user. Please run `sudo MiniFTPD`" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!configure::parse_command_parameter(argc, argv)) {
        std::cout << "MiniFTPD Need a configure file.\n";
        configure::show_usage();
        exit(EXIT_FAILURE);
    }

    /* 解析配置文件中的参数 */
    configure::parse_config_file();

    CLTCPServer tcp_server(configure::SERVER_LISTEN_HOST.c_str(), configure::SERVER_LISTEN_PORT);
    int listen_fd = tcp_server.start_listen();

    if (configure::FORCE_PASSIVE_SERVER_IP.empty()) {
        if (tcp::get_local_ip(nullptr) < 0) {
            std::cerr << " Get local IP address failed. Please configure force passive IP" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    CLEpoll event_epoll(5);
    /* 通过管道统一信号源  */
    CLPipeWrapper signal_pipe(signal_pipe_fd);
    signal_pipe.set_write_nonblock();

    /* 添加 Epoll IO */
    event_epoll.add_event_fd(listen_fd);
    event_epoll.add_event_fd(signal_pipe.get_read_fd());

    CLSignalWrapper signal_handle;
    signal_handle.add_signal(SIGCHLD, signal_handler);
    signal_handle.add_signal(SIGTERM, signal_handler);
//    signal_handle.add_signal(SIGINT, signal_handler);
    signal_handle.ignore_signal(SIGPIPE);

    std::unordered_map<pid_t , unsigned int> client_process_mapper;  /* client 进程到 ip 的映射 */

    bool terminate = false;
    while (!terminate) {
        event_epoll.wait_for_event();
        while (event_epoll.has_event()) {
            auto event = event_epoll.next_event();
            /* 新的客户连接 */
            int socket_fd = CLEpoll::get_event_fd(event);
            if (socket_fd == listen_fd) {
                struct sockaddr_in client_addr{0};
                int connect_fd = tcp_server.accept_client(client_addr);
                if (connect_fd < 0) {
                    perror("Accept client error");
                    continue;
                }

                unsigned int client_ip = client_addr.sin_addr.s_addr;
                utility::debug_info(std::string("Accept client: " + std::to_string(client_ip)));

                /* 限流 */
                if (tcp_server.limit_client_crowding(connect_fd, client_ip)) {
                    close(connect_fd);
                    continue;
                }

                /* 创建服务进程，为 client 服务 */
                pid_t pid = fork();
                if (pid == -1) {
                    perror("Fork new process error");
                    continue;
                }

                if (pid == 0) {
                    /* 父进程退出，给子进程发送 SIGTERM 信号 */
                    prctl(PR_SET_PDEATHSIG, SIGTERM);
                    close(listen_fd);

                    CLFtpHandler ftp_handler(connect_fd);
                    ftp_handler.start_handle();

                    exit(0);
                } else {
                    /* 父进程 */
                    close(connect_fd);
                    client_process_mapper[pid] = client_ip;
                    utility::debug_info(std::string("Forked process: ") + std::to_string(pid));
                }

            } else if ((socket_fd == signal_pipe.get_read_fd()) && CLEpoll::is_in_event(event)) {
                /* 处理信号事件 */
                char signals[1024] = {0};
                int ret = signal_pipe.get_a_signal(signals, sizeof(signals));
                if (ret == -1 || ret == 0) {
                    continue;
                } else {
                    for (int i = 0; i < ret; ++i) {
                        switch (signals[i]) {
                            /* 子进程退出，表示有某个客户端关闭了连接 */
                            case SIGCHLD: {
                                pid_t pid;
                                int stat;
                                while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
                                    /* 进程退出后的善后处理 */
                                    utility::debug_info(std::string("Subprocess: ") + std::to_string(pid) + " exited");
                                    tcp_server.on_a_client_exit(client_process_mapper[pid]);
                                }
                                break;
                            }
                            case SIGKILL:
                            case SIGTERM:
                            case SIGINT: {
                                /* 结束服务器程序 */
                                terminate = true;
                                utility::debug_info(std::string("Process Terminated."));
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}