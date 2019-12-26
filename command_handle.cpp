//
// Created by MLee on 2019/12/24.
//

#include <functional>
#include <cstdarg>
#include <algorithm>
#include "command_handle.h"
#include "utility.h"
#include "tcp.h"

CLCommandHandle::CLCommandHandle(int command_fd, int read_pipe_fd) :
        m_pipe_fd(read_pipe_fd), m_cmd_fd(command_fd), m_b_stop(false) {
    m_client_cmd_vec.clear();
    memset(client_cmd_line, 0, sizeof(client_cmd_line));

    // 访问控制命令
    m_cmd_exec_map["USER"] = &CLCommandHandle::do_user;
    m_cmd_exec_map["PASS"] = &CLCommandHandle::do_pass;
    m_cmd_exec_map["CWD"] = &CLCommandHandle::do_cwd;
    m_cmd_exec_map["CDUP"] = &CLCommandHandle::do_cdup;
    m_cmd_exec_map["QUIT"] = &CLCommandHandle::do_quit;
    // 传输参数命令
    m_cmd_exec_map["PORT"] = &CLCommandHandle::do_port;
    m_cmd_exec_map["PASV"] = &CLCommandHandle::do_pasv;
    m_cmd_exec_map["TYPE"] = &CLCommandHandle::do_type;
    // FTP 服务命令
    m_cmd_exec_map["RETR"] = &CLCommandHandle::do_retr;
    m_cmd_exec_map["STOR"] = &CLCommandHandle::do_stor;
    m_cmd_exec_map["REST"] = &CLCommandHandle::do_rest;
    m_cmd_exec_map["RNFR"] = &CLCommandHandle::do_rnfy;
    m_cmd_exec_map["RNTO"] = &CLCommandHandle::do_rnto;
    m_cmd_exec_map["ABOR"] = &CLCommandHandle::do_abor;
    m_cmd_exec_map["DELE"] = &CLCommandHandle::do_delete;
    m_cmd_exec_map["RMD"] = &CLCommandHandle::do_rmd;
    m_cmd_exec_map["MKD"] = &CLCommandHandle::do_mkd;
    m_cmd_exec_map["PWD"] = &CLCommandHandle::do_pwd;
    m_cmd_exec_map["LIST"] = &CLCommandHandle::do_list;
    m_cmd_exec_map["NLST"] = &CLCommandHandle::do_nlst;
    m_cmd_exec_map["SITE"] = &CLCommandHandle::do_site;
    m_cmd_exec_map["SYST"] = &CLCommandHandle::do_syst;
    m_cmd_exec_map["HELP"] = &CLCommandHandle::do_help;
    m_cmd_exec_map["NOOP"] = &CLCommandHandle::do_noop;
    // 其他
    m_cmd_exec_map["FEAT"] = &CLCommandHandle::do_feat;
    m_cmd_exec_map["OPTS"] = &CLCommandHandle::do_opts;
    m_cmd_exec_map["SIZE"] = &CLCommandHandle::do_size;
}

CLCommandHandle::~CLCommandHandle() {
    utility::debug_info("Close command link");
    close(m_cmd_fd);
    close(m_pipe_fd);
}


void CLCommandHandle::do_user() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_user()");
}

void CLCommandHandle::do_pass() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_pass()");
}

void CLCommandHandle::do_cwd() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_cwd()");
}

void CLCommandHandle::do_cdup() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_cdup()");
}

void CLCommandHandle::do_quit() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_quit()");
}

void CLCommandHandle::do_port() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_port()");
}

void CLCommandHandle::do_pasv() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_pasv()");
}

void CLCommandHandle::do_type() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_type()");
}

void CLCommandHandle::do_retr() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_retr()");
}

void CLCommandHandle::do_stor() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_stor()");
}

void CLCommandHandle::do_rest() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_rest()");
}

void CLCommandHandle::do_rnfy() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_rnfy()");
}

void CLCommandHandle::do_rnto() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_rnto()");
}

void CLCommandHandle::do_abor() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_abor()");
}

void CLCommandHandle::do_delete() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_delete()");
}

void CLCommandHandle::do_rmd() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_rmd()");
}

void CLCommandHandle::do_mkd() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_mkd()");
}

void CLCommandHandle::do_pwd() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_pwd()");
}

void CLCommandHandle::do_list() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_list()");
}

void CLCommandHandle::do_nlst() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_nlst()");
}

void CLCommandHandle::do_site() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_site()");
}

void CLCommandHandle::do_syst() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_syst()");
}

void CLCommandHandle::do_help() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_help()");
}

void CLCommandHandle::do_noop() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_noop()");
}

void CLCommandHandle::do_feat() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_feat()");
}

void CLCommandHandle::do_opts() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_opts()");
}

void CLCommandHandle::do_size() {
    utility::debug_socket_info(m_cmd_fd, "server execute do_size()");
}

void CLCommandHandle::handle() {
    welcome_client();
    fcntl(m_cmd_fd, F_SETOWN, getpid());

    while (!m_b_stop) {
        memset(client_cmd_line, 0, sizeof(client_cmd_line));
        m_client_cmd_vec.clear();

        /* 获取用户命令 */
        if (!get_cmd_line(client_cmd_line, sizeof(client_cmd_line))) {
            utility::debug_info("Get client command error");
        } else {
            utility::debug_info(std::string("Get user command: ") + client_cmd_line + "**");
        }

        /* 解析命令 */
        parse_cmd(client_cmd_line);

        /* 调用相应的函数，执行命令 */
        std::string cmd = m_client_cmd_vec[0];
        utility::debug_info(std::string("User command: ") + cmd + "**");
        if (m_cmd_exec_map.count(cmd) == 0) {
            reply_client("%d Unknown command.", ftp_response_code::kFTP_BADCMD);
        } else {
            /* 通过成员函数指针调用成员函数 */
            (this->*m_cmd_exec_map[cmd])();
        }
    }

}

void CLCommandHandle::reply_client(const char *format, ...) {
    char buf[1024] = {0};
    va_list v;
    va_start(v, format);
    vsnprintf(buf, sizeof(buf), format, v);
    va_end(v);

    /* 添加上 \r\n */
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s", kCRLF);

    tcp::send_data(m_cmd_fd, buf, strlen(buf));
}


void CLCommandHandle::welcome_client() {
    reply_client("%d Welcome to Mini FTP server", ftp_response_code::kFTP_GREET);
}

bool CLCommandHandle::get_cmd_line(char *buf, size_t len) {
    int ret = 0;
    /* TODO: 以下为简化版的实现，待完善*/
    if ((ret = recv(m_cmd_fd, buf, len, 0)) < 0) {
        utility::unix_error("Recv error");
    } else {
        return true;
    }

    return false;
}

void CLCommandHandle::parse_cmd(char *cmd_line) {
    /* TODO: 以下为简化版的实现，待完善 */
    std::string str_cmd = std::string(cmd_line);
    /* 将命令转换为大写 */
    std::transform(str_cmd.begin(), str_cmd.end(), str_cmd.begin(), toupper);
    /* 最粗暴的去除行尾回车换行符，待完善 */
    if (str_cmd[str_cmd.size() - 1] == '\n') {
        str_cmd.erase(str_cmd.size() - 1);
    }
    if (str_cmd[str_cmd.size() - 1] == '\r') {
        str_cmd.erase(str_cmd.size() - 1);
    }
    m_client_cmd_vec.emplace_back(str_cmd);
}







