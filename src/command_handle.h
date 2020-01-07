//
// Created by MLee on 2019/12/24.
//

#ifndef MINIFTPD_COMMAND_HANDLE_H
#define MINIFTPD_COMMAND_HANDLE_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <set>
#include "ftp_codes.h"
#include "buffer.h"
#include "common.h"
#include "ipc_utility.h"
#include "tcp.h"

class CLCommandHandle {
public:
    CLCommandHandle(int command_fd, int read_pipe_fd);

    ~CLCommandHandle();

    void handle();

private:
    void reply_client(const char *format, ...);

    void welcome_client();  /* 给出 welcome 信息 */

    bool get_cmd_line(char *buf, size_t len);   /* 获取一行命令 */
    void parse_cmd(char *cmd_line); /* 解析用户命令 */
    static void strip_crlf(std::string &str); /* 去除行尾 \r\n */
    void split_cmd_and_argv(std::string &str, const std::string &delimiter);

    const std::string &get_command() const {    /* 获取命令 */
        if (m_client_cmd_vec.empty()) {
            return k_empty_string;
        }
        return m_client_cmd_vec[0];
    }

    const std::string &get_a_cmd_argv() const {     /* 获取命令的参数 */
        if (m_client_cmd_vec.size() < 2) {
            return k_empty_string;
        }
        return m_client_cmd_vec[1];
    }

    inline static bool is_file_existed(const char *file_name);

private:
    /* 访问控制命令 */
    void do_user(); /* 用户名 */
    void do_pass(); /* 密码  */
    void do_cwd();  /* 改变工作目录 */
    void do_cdup(); /* 返回上层目录 */
    void do_quit(); /* 注销用户 */
    /* 传输参数命令 */
    void do_port(); /* 指定数据连接时的主机数据端口 */
    void do_pasv(); /* 被动模式 */
    void do_type(); /* 表示类型 */
    /* FTP 服务命令 */
    void do_retr(); /* 获取某个文件 */
    void do_stor(); /* 保存（上传）文件 */
    void do_rest(); /* 重新开始（实现断点续传）*/
    void do_rnfr(); /* 重命名开始 */
    void do_rnto(); /* 重命名为（两个命令一起为文件重新命名）  */
    void do_abor(); /* 放弃 */
    void do_delete();   /* 删除文件 */
    void do_rmd();  /* 删除目录 */
    void do_mkd();  /* 新建目录 */
    void do_pwd();  /* 打印工作目录 */
    void do_list(); /* 列出文件信息 */
    void do_nlst(); /* 列出名字列表 */
    void do_site(); /* 站点参数 */
    void do_syst(); /* 操作系统类型 */
    void do_help(); /* 帮助信息 */
    void do_noop(); /* 空操作 */
    /* 其他 */
    void do_feat(); /* 处理Feat命令 */
    void do_opts(); /* 调整选项 */
    void do_size(); /* 获取文件的大小 */
private:
    void do_chmod(unsigned int perm, const char *file_name);

    void do_unmask(unsigned int mask);

    ipc_utility::EMState do_send_list(bool verbose);

    std::shared_ptr<tcp::CLConnection> get_data_connection();

private:
    size_t send_ipc_msg(const void *msg, int len) { /* 发送 ipc 信息给子进程 */
        return send(m_pipe_fd, msg, len, 0);
    }

    size_t recv_ipc_msg(void *msg, int len) { /* 接收来自子进程的 ipc 信息 */
        return recv(m_pipe_fd, msg, len, 0);
    }

private:
    const char *kCRLF = "\r\n";
    const std::string k_SP_delimiter = " "; /* 命令的空格分隔符 */
    const std::string k_empty_string = "";  /* 空字符串 */
    const int k_new_dir_perm = 0777;    /* 新建的目录的默认权限 */
    const int k_uploaded_file_perm = 0666;    /* 上传的文件的默认权限 */
//    typedef void (CLCommandHandle::*PCommandExecutor)();
    using PCommandExecutor = void (CLCommandHandle::*)();

private:
    std::unordered_map<std::string, PCommandExecutor> m_cmd_exec_map;
    std::set<std::string> m_cmd_need_auth;
    bool m_b_authored;
    int m_cmd_fd;
    int m_pipe_fd;

    bool m_b_stop;
    std::vector<std::string> m_client_cmd_vec;
    char client_cmd_line[MAX_COMMAND_LINE]; /* 用户读取用户命令行的缓冲区 */

    CLBuffer m_buffer;  /* 用于接收客户数据的 socket 缓冲区 */

    enum EMMode {
        ASCII, /* ascii文本格式 */
        BINARY, /* 二进制格式 */
    };
    EMMode m_data_type; /* 传输的数据类型 */

    std::string m_file_name; /* 用来记录需要重命名的文件名 */
    static const int BYTES_PEER_TRANSFER = 1024 * 1024; /* 文件下载时每次传输的数据量 */
    long long m_resume_point;   /* 断点续传点 */
private:
    uid_t m_uid;    /* 用户 id  */
};

#endif //MINIFTPD_COMMAND_HANDLE_H
