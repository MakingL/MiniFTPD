//
// Created by MLee on 2019/12/24.
//

#include <functional>
#include <cstdarg>
#include <algorithm>
#include "command_handle.h"
#include "utility.h"
#include "tcp.h"
#include "ipc_utility.h"
#include "record_lock.h"
#include <unistd.h>
#include <crypt.h>
#include "speed_barrier.h"
#include "configure.h"

CLCommandHandle::CLCommandHandle(int command_fd, int read_pipe_fd) :
        m_pipe_fd(read_pipe_fd), m_cmd_fd(command_fd), m_b_stop(false),
        m_data_type(BINARY), m_resume_point(0), m_b_authored(false) {

    m_client_cmd_vec.clear();
    m_cmd_need_auth.clear();
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
    m_cmd_exec_map["RNFR"] = &CLCommandHandle::do_rnfr;
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

    m_cmd_need_auth = {"CWD", "CDUP", "PORT", "PASV", "TYPE",
                       "RNFR", "RNTO", "DELE", "RMD", "MKD",
                       "PWD", "LIST", "NLST", "FEAT", "SIZE"};
}

CLCommandHandle::~CLCommandHandle() {
    utility::debug_info("Close command link");
    close(m_cmd_fd);
    close(m_pipe_fd);
}


void CLCommandHandle::do_user() {
    utility::debug_info("server execute do_user()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: user name", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    struct passwd *pw = getpwnam(cmd_argv.c_str());
    if (pw == nullptr) {
        // 用户不存在
        reply_client("%d Login incorrect.", ftp_response_code::kFTP_LOGINERR);
        return;
    }

    m_uid = pw->pw_uid;

    reply_client("%d Please specify the password.", ftp_response_code::kFTP_GIVEPWORD);
}

void CLCommandHandle::do_pass() {
    utility::debug_info("server execute do_pass()");

    struct passwd *pw = getpwuid(m_uid);
    if (pw == nullptr) {
        // 用户不存在
        reply_client("%d Login incorrect.", ftp_response_code::kFTP_LOGINERR);
        return;
    }

    struct spwd *sp = getspnam(pw->pw_name);
    if (sp == nullptr) {
        reply_client("Login incorrect.", ftp_response_code::kFTP_LOGINERR);
        return;
    }

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: password", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    // 将明文进行加密
    char *encrypted_pass = crypt(cmd_argv.c_str(), sp->sp_pwdp);
    // 验证密码
    if (strcmp(encrypted_pass, sp->sp_pwdp) != 0) {
        reply_client("%d Login incorrect.", ftp_response_code::kFTP_LOGINERR);
        return;
    }

    /*signal(SIGURG, handle_sigurg);
    activate_sigurg(sess->ctrl_fd);*/

    /*umask(077);*/
    setegid(pw->pw_gid);
    seteuid(pw->pw_uid);
    chdir(pw->pw_dir);
    m_b_authored = true;
    reply_client("%d Login successful.", ftp_response_code::kFTP_LOGINOK);
}

void CLCommandHandle::do_cwd() {
    utility::debug_info("server execute do_cwd()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Need target path name.", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    if (chdir(cmd_argv.c_str()) < 0) {
        reply_client("%d Failed to change directory!", ftp_response_code::kFTP_NOPERM);
    } else {
        reply_client("%d Working directory changed to \"%s\"", ftp_response_code::kFTP_CWDOK, cmd_argv.c_str());
    }
}

void CLCommandHandle::do_cdup() {
    utility::debug_info("server execute do_cdup()");

    if (chdir("..") < 0) {
        reply_client("%d Failed to change directory.", ftp_response_code::kFTP_NOPERM);
        return;
    } else {
        reply_client("%d Directory successfully changed.\r\n", ftp_response_code::kFTP_CWDOK);
    }
}

void CLCommandHandle::do_quit() {
    utility::debug_info("server execute do_quit()");

    reply_client("%d Goodbye.", ftp_response_code::kFTP_GOODBYE);
    m_b_stop = true;
}

void CLCommandHandle::do_port() {
    utility::debug_info("server execute do_port()");
    /* 这个参数是用来指定数据连接时的主机数据端口 */
    /* 客户端传递过来用于数据传送的ip地址和端口号,然后这个函数将其传递给另外一个进程 */
    //PORT 192,168,0,100,123,233

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Need host information.", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    int ip[4] = {0};
    unsigned int h_port, l_port;
    if (sscanf(cmd_argv.c_str(), "%d,%d,%d,%d,%u,%u", &ip[0], &ip[1], &ip[2], &ip[3], &h_port, &l_port) < 6) {
        reply_client("%d Bad host information.", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    char ip_addr[20] = {0};
    snprintf(ip_addr, sizeof(ip_addr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]); /* 获得字符类型的ip */
    uint16_t port = (h_port << 8) | (l_port & 0xFF); /* 得到port */

    utility::debug_info(std::string("PORT client IP: ") + ip_addr + " port: " + std::to_string(port));

    /* 将地址发送给另外一个进程 */
    ipc_utility::EMIpcCmd cmd = ipc_utility::k_PeerInfo;
    send_ipc_msg(&cmd, sizeof(cmd)); /* 传递命令 */
    send_ipc_msg(&port, sizeof(port)); /* 传递端口号 */
    send_ipc_msg(ip_addr, strlen(ip_addr)); /* 传递 IP 地址 */
    reply_client("%d PORT command successful. Consider using PASV.", ftp_response_code::kFTP_PORTOK);
}

void CLCommandHandle::do_pasv() {
    /* PASV 模式代表客户端主动来连接服务器,双方来协商一些连接的信息 */
    utility::debug_info("server execute do_pasv()");

    ipc_utility::EMIpcCmd cmd = ipc_utility::k_ExecPASV;
    tcp::send_data(m_pipe_fd, &cmd, sizeof(cmd));

    ipc_utility::EMState state = ipc_utility::Error;
    recv_ipc_msg(&state, sizeof(state));
    if (state != ipc_utility::Success) {


        if (state == ipc_utility::PortExhausted) {
            utility::debug_info(std::string("PASV error: All reserved TCP ports are busy"));

            reply_client("%d All reserved TCP ports are busy.", ftp_response_code::kFTP_BADSENDCONN);
        } else {
            utility::debug_info(std::string("PASV error: Can't open data connection"));

            reply_client("%d Can't open data connection.", ftp_response_code::kFTP_BADSENDCONN);
        }
        return;
    }

    int ip_size;
    char ip_addr[20] = {0};
    uint16_t port;
    tcp::recv_data(m_pipe_fd, &ip_size, sizeof(ip_size));    /* 得到ip地址的长度 */
    tcp::recv_data(m_pipe_fd, ip_addr, ip_size);    /* 获得ip地址 */
    tcp::recv_data(m_pipe_fd, &port, sizeof(port));  /* 获得端口地址 */

    utility::debug_info(std::string("PASV ip_addr: ") + ip_addr + " port: " + std::to_string(port));

    unsigned int ip_v[4];
    if (sscanf(ip_addr, "%u.%u.%u.%u", ip_v, ip_v + 1, ip_v + 2, ip_v + 3) < 4) {
        utility::debug_info("IP Address format is wrong");
    }
    reply_client("%d Entering Entering Passive Mode (%u,%u,%u,%u,%u,%u).", ftp_response_code::kFTP_PASVOK,
                 ip_v[0], ip_v[1], ip_v[2], ip_v[3], port >> 8, port & 0xFF);
}

void CLCommandHandle::do_type() {
    utility::debug_info("server execute do_type()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument\n A(scii) I(mage) L(ocal)", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    /* 转换为大写 */
    std::transform(cmd_argv.begin(), cmd_argv.end(), cmd_argv.begin(), toupper);

    if (cmd_argv == "A") {
        m_data_type = ASCII;
        reply_client("%d Switching to ASCII mode.", ftp_response_code::kFTP_TYPEOK);
    } else if (cmd_argv == "I") {
        m_data_type = BINARY;
        reply_client("%d Switching to BINARY mode.", ftp_response_code::kFTP_TYPEOK);
    } else {
        reply_client("%d Unrecognised TYPE command.", ftp_response_code::kFTP_BADCMD);
    }
}

void CLCommandHandle::do_retr() {   /* 下载文件 */
    utility::debug_info("server execute do_retr()");

    std::shared_ptr<tcp::CLConnection> data_connection = get_data_connection();  /* 数据连接 */

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: target file name", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    int fd = open(cmd_argv.c_str(), O_RDONLY, NULL); /* 打开文件 */
    if (fd < 0) {
        reply_client("%d Open file failed.", ftp_response_code::kFTP_FILEFAIL);
        return;
    }

    /* 开始传送文件 */
    struct stat file_info{0};
    size_t file_size = 0;
    {   /* 代码块外自动释放文件锁 */
        CLRDRecordLock file_rd_lock(fd); /* 读锁,如果不能读的话,会一直阻塞 */
        fstat(fd, &file_info);

        if (!S_ISREG(file_info.st_mode)) { /* 判断是否为普通文件,设备文件不能下载 */
            reply_client("%d It is not a regular file.", ftp_response_code::kFTP_FILEFAIL);
            return;
        }

        file_size = (long long) file_info.st_size;  /* 文件大小 */
        if (m_data_type == BINARY) {    /* 数据类型 */
            reply_client("%d Opening BINARY mode data connection for %s (%lld bytes).",
                         ftp_response_code::kFTP_DATACONN, cmd_argv.c_str(), file_size);
        } else {
            reply_client("%d Opening ASCII mode data connection for %s (%lld bytes).",
                         ftp_response_code::kFTP_DATACONN, cmd_argv.c_str(), file_size);
        }

        if (m_resume_point != 0) {  /* 断点续传支持 */
            if ((lseek(fd, m_resume_point, SEEK_SET)) < 0) { /* 重定位文件 */
                utility::debug_info("lseek file error");
            }
            file_size -= m_resume_point; /* 需要传送的字节的数目 */
        }

        CLSpeedBarrier download_speed_barrier(configure::MAX_DOWNLOAD_SPEED);   /* 限速器 */
        while (file_size > 0) { /* 传输文件数据 */
            int sent_size = sendfile(data_connection->get_fd(), fd, nullptr, BYTES_PEER_TRANSFER); /* 每次发送一点 */
            if (sent_size == -1) {
                utility::debug_info("Sent file failed");
                break;
            }
            file_size -= sent_size;
            download_speed_barrier.limit_speed(sent_size);  /* 限速 */
        }
    }
    if (file_size == 0) {
        reply_client("%d Transfer complete.", ftp_response_code::kFTP_TRANSFEROK);
    } else {    /* 连接关闭,放弃传输 */
        reply_client("%d Transfer failed.", ftp_response_code::kFTP_BADSENDNET);
    }

    tcp::close_fd(fd); /** 关闭文件 */
    m_resume_point = 0; /* 文件已经传送完毕， 要将断点复原 */
}

void CLCommandHandle::do_stor() {
    utility::debug_info("server execute do_stor()");

    std::shared_ptr<tcp::CLConnection> data_connection = get_data_connection();  /* 数据连接 */

    auto cmd_argv = get_a_cmd_argv();   /* 获取文件名参数 */
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: target file name", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    /* store的话代表不用append */
    int fd = open(cmd_argv.c_str(), O_CREAT | O_WRONLY, k_uploaded_file_perm);
    if (fd < 0) {
        reply_client("%d Can't open that file.", ftp_response_code::kFTP_UPLOADFAIL);
        return;
    }

    ipc_utility::EMState state = ipc_utility::Success;

    {   /* 跳出代码块自动释放文件写锁 */
        CLWRRecordLock lock(fd); /* 对文件加写锁 */

        if (m_resume_point == 0) {
            ftruncate(fd, 0); /* 清空文件内容 */
            if ((lseek(fd, 0, SEEK_SET)) < 0) { /* 定位到文件的开头 */
                utility::debug_info("lseek file error");
            }
        } else {  /* 断点续传支持 */
            if ((lseek(fd, m_resume_point, SEEK_SET)) < 0) { /* 定位到文件的偏移位置 */
                utility::debug_info("lseek file error");
            }
        }

        reply_client("%d OK to send data.", ftp_response_code::kFTP_DATACONN);

        /* 读取数据,写入本地文件 */
        char buf[1024 * 1024] = {0};
        CLSpeedBarrier upload_speed_barrier(configure::MAX_UPLOAD_SPEED);   /* 限速器 */
        while (true) {
            int res = read(data_connection->get_fd(), buf, sizeof(buf));
            if (res == -1) {
                if (errno == EINTR)
                    continue;
                else {
                    state = ipc_utility::ReadError;
                    break;
                }
            } else if (res == 0) { /* 对方关闭了连接 */
                break;
            }
            if (write(fd, buf, res) != res) { /* 往文件中写入数据 */
                state = ipc_utility::WriteError;
                break;
            }
            upload_speed_barrier.limit_speed(res);  /* 限速 */
        }
    }
    tcp::close_fd(fd); /* 关闭文件 */

    if (state == ipc_utility::ReadError) { /* 读出错 */
        reply_client("%d Fail to read from network stream.", ftp_response_code::kFTP_BADSENDNET);
    } else if (state == ipc_utility::WriteError) {    /* 写出错 */
        reply_client("%d Fail to write to local file.", ftp_response_code::kFTP_BADSENDFILE);
    } else {
        reply_client("%d Transfer complete.", ftp_response_code::kFTP_TRANSFEROK); /* 文件传送成功 */
    }
}

void CLCommandHandle::do_rest() {
    utility::debug_info("server execute do_rest()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: resume point", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    m_resume_point = std::atoll(cmd_argv.c_str());  /* 断点续传支持 */
    reply_client("%d Restart position accepted.", ftp_response_code::kFTP_RESTOK);
}

void CLCommandHandle::do_rnfr() {
    utility::debug_info("server execute do_rnfr()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: file name", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    if (!is_file_existed(cmd_argv.c_str())) {
        reply_client("%d Sorry, but that file doesn't exist.", ftp_response_code::kFTP_FILEFAIL);
        return;
    }

    /* 这个命令指定了需要重新命名的原始路径名,后面必须马上接着RNTO命令,来指定新的文件路径 */
    m_file_name = std::move(cmd_argv);
    reply_client("%d Ready for rename file.", ftp_response_code::kFTP_RNFROK);
}

void CLCommandHandle::do_rnto() {
    utility::debug_info("server execute do_rnto()");

    if (m_file_name.empty()) {
        reply_client("%d RNFR require first.", ftp_response_code::kFTP_NEEDRNFR);
        return;
    }

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: file name", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    if (rename(m_file_name.c_str(), cmd_argv.c_str()) < 0) {    /* 重命名 */
        reply_client("%d Rename error.", ftp_response_code::kFTP_RENAMEFAIL);
        return;
    }

    reply_client(R"(%d Successfully rename file "%s" to "%s".)",
                 ftp_response_code::kFTP_RENAMEOK, m_file_name.c_str(), cmd_argv.c_str());
    m_file_name.clear(); /* 每次重命名完要清空 m_file_name*/
}

void CLCommandHandle::do_abor() {
    utility::debug_info("server execute do_abor()");

    /*
     * 服务器接收这个命令时可能处在两种状态.(1) FTP服务命令已经完成了,或者(2)FTP服务还在执行中.
     * 对于第一种情况，服务器关闭数据连接(如果数据连接是打开的)回应226代码,表示放弃命令已经成功处理.
     * 对于第二种情况,服务器放弃正在进行的FTP服务,关闭数据连接,返回426响应代码,表示请求服务异常终止,
     * 然后服务器发送226代码,表示放弃命令成功处理.
     *
     * 在这里的ABOR命令中,只发送了226代码,因为426代码在其余各个服务中发送.
     */
    reply_client("%d No transfer to Abort!", ftp_response_code::kFTP_TRANSFEROK);
}

void CLCommandHandle::do_delete() {
    utility::debug_info("server execute do_delete()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing argument: file name", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    if (unlink(cmd_argv.c_str()) < 0) {
        reply_client("%d Delete operation fail.", ftp_response_code::kFTP_NOPERM);
        return;
    }

    reply_client("%d Delete operation successful.", ftp_response_code::kFTP_DELEOK);
}

void CLCommandHandle::do_rmd() {
    utility::debug_info("server execute do_rmd()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Need target path name.", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    if (rmdir(cmd_argv.c_str()) < 0) {
        reply_client("%d Remove a directory failed.", ftp_response_code::kFTP_FILEFAIL);
        return;
    } else {
        reply_client("%d Remove successfully.", ftp_response_code::kFTP_RMDIROK);
    }
}

void CLCommandHandle::do_mkd() {
    utility::debug_info("server execute do_mkd()");
    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Need new path name.", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    if (mkdir(cmd_argv.c_str(), k_new_dir_perm) < 0) {
        reply_client("%d Create directory operation failed.", ftp_response_code::kFTP_FILEFAIL);
        return;
    }
    reply_client("%d \"%s\" directory created.", ftp_response_code::kFTP_MKDIROK, cmd_argv.c_str());
}

void CLCommandHandle::do_pwd() {
    utility::debug_info("server execute do_pwd()");

    char pwd[MAX_PATH_LEN] = {0};
    if (getcwd(pwd, sizeof(pwd)) == nullptr) {
        utility::debug_info("Get PWD path failed");
        reply_client("%d Fail to get pwd", ftp_response_code::kFTP_BADSENDFILE);
    } else {
        reply_client("%d \"%s\"", ftp_response_code::kFTP_PWDOK, pwd);
    }
}

void CLCommandHandle::do_list() {
    utility::debug_info("server execute do_list()");

    reply_client("%d Here comes the directory listing.", ftp_response_code::kFTP_DATACONN);

    /* 开始传输列表 */
//    auto cmd_argv = get_a_cmd_argv();
//    bool verbose = cmd_argv.empty(); /* 是否要传递所有的细节 */
    if (ipc_utility::Error == do_send_list(true)) {
        reply_client("%d Directory send not OK.", ftp_response_code::kFTP_NOPERM); /* 没有权限 */
    } else {
        reply_client("%d Directory send OK.", ftp_response_code::kFTP_TRANSFEROK);
    }
}

void CLCommandHandle::do_nlst() {
    utility::debug_info("server execute do_nlst()");

    reply_client("%d Here comes the directory listing.", ftp_response_code::kFTP_DATACONN);

    /* 开始传输列表 */
    if (ipc_utility::Error == do_send_list(false)) {
        reply_client("%d Directory send not OK.", ftp_response_code::kFTP_NOPERM); /* 没有权限 */
    } else {
        reply_client("%d Directory send OK.", ftp_response_code::kFTP_TRANSFEROK);
    }
}

void CLCommandHandle::do_site() {
    utility::debug_info("server execute do_site()");

    std::string cmd_argv_head = get_a_cmd_argv();
    /* 转换为大写 */
    std::transform(cmd_argv_head.begin(), cmd_argv_head.end(), cmd_argv_head.begin(), toupper);
    if (m_client_cmd_vec.size() == 4 && cmd_argv_head == "CHMOD") {
        /*SITE CHMOD <perm> <file>*/
        do_chmod(atoi(m_client_cmd_vec[2].c_str()), m_client_cmd_vec[3].c_str());
    } else if (m_client_cmd_vec.size() == 3 && cmd_argv_head == "UMASK") {
        /* SITE UMASK [umask] */
        do_unmask(atoi(m_client_cmd_vec[2].c_str()));
    } else {
        reply_client("%d Unknown site command.", ftp_response_code::kFTP_BADCMD);
    }
}

void CLCommandHandle::do_syst() {
    utility::debug_info("server execute do_syst()");

    reply_client("%d Unix Type: L8", ftp_response_code::kFTP_SYSTOK);
}

void CLCommandHandle::do_help() {
    utility::debug_info("server execute do_help()");

    reply_client("%d-The following commands are recognized.", ftp_response_code::kFTP_HELP);
    std::string cmd;
    for (const auto &cmd_pair: m_cmd_exec_map) {
        cmd += " " + cmd_pair.first;
    }
    reply_client(cmd.c_str());
    reply_client("%d Help OK.", ftp_response_code::kFTP_HELP);
}

void CLCommandHandle::do_noop() {
    utility::debug_info("server execute do_noop()");

    /* NOOP 指令只要发送 ok 就行了 */
    reply_client("%d NOOP OK.", ftp_response_code::kFTP_NOOPOK);
}

void CLCommandHandle::do_feat() {
    utility::debug_info("server execute do_feat()");

    reply_client("%d-Features:", ftp_response_code::kFTP_FEAT);
    reply_client(" EPRT");
    reply_client(" EPSV");
    reply_client(" MDTM");
    reply_client(" PASV");
    reply_client(" REST STREAM"); /* 断点续传 */
    reply_client(" SIZE");
    reply_client(" TVFS");
    reply_client(" UTF8");
    reply_client("%d End", ftp_response_code::kFTP_FEAT);
}

void CLCommandHandle::do_opts() {
    utility::debug_info("server execute do_opts()");

    reply_client("%d All is well!", ftp_response_code::kFTP_OPTSOK); /* 直接选择OK! */
}

void CLCommandHandle::do_size() {
    utility::debug_info("server execute do_size()");

    auto cmd_argv = get_a_cmd_argv();
    if (cmd_argv.empty()) {
        reply_client("%d Missing file name argument.", ftp_response_code::kFTP_BADOPTS);
        return;
    }

    struct stat fileInfo{0};
    if (stat(cmd_argv.c_str(), &fileInfo) < 0) {
        reply_client("%d SIZE operation failed.", ftp_response_code::kFTP_FILEFAIL);
        return;
    }

    if (!S_ISREG(fileInfo.st_mode)) { /* 没有权限获得文件的信息 */
        reply_client("%d Have no permission.", ftp_response_code::kFTP_NOPERM);
        return;
    }
    char buf[20] = {0};
    sprintf(buf, "%lld", (long long) fileInfo.st_size);

    reply_client("%d %s", ftp_response_code::kFTP_SIZEOK, buf);
}

void CLCommandHandle::do_chmod(unsigned int perm, const char *file_name) {
    if (chmod(file_name, perm) < 0) {
        reply_client("%d SITE CHMOD command failed.", ftp_response_code::kFTP_BADCMD);
        return;
    }
    reply_client("%d SITE CHMOD Command OK.");
}

void CLCommandHandle::do_unmask(unsigned int mask) {
    umask(mask);
    reply_client("%d Umask OK.", ftp_response_code::kFTP_UMASKOK);
}


void CLCommandHandle::handle() {
    welcome_client();
    fcntl(m_cmd_fd, F_SETOWN, getpid());

    while (!m_b_stop) {
        memset(client_cmd_line, 0, sizeof(client_cmd_line));
        m_client_cmd_vec.clear();

        /* 获取用户命令 */
        if (!get_cmd_line(client_cmd_line, sizeof(client_cmd_line))) {
            utility::debug_info("Conn't Get client command line");
            continue;
        } else {
            utility::debug_info(std::string("Get user command: ") + client_cmd_line + "**");
        }

        /* 解析命令 */
        parse_cmd(client_cmd_line);

        /* 调用相应的函数，执行命令 */
        std::string cmd = get_command();
        if (cmd.empty()) {
            utility::debug_info(std::string("Server get empty command."));
            continue;
        }
        utility::debug_info(std::string("User command: ") + cmd + "**");
        if (m_cmd_exec_map.count(cmd) == 0) {
            reply_client("%d Unknown command.", ftp_response_code::kFTP_BADCMD);
        } else if (!m_b_authored && m_cmd_need_auth.count(cmd)) {
            /* 尚未登录，且该命令需要登录后才能被执行 */
            reply_client("%d You aren't logged in.", ftp_response_code::kFTP_LOGINERR);
        } else {
            /* 通过成员函数指针调用成员函数 */
            (this->*m_cmd_exec_map[cmd])();
        }
    }

    utility::debug_info("Command link process exit");
    /* 通知子进程退出 */
    ipc_utility::EMIpcCmd cmd = ipc_utility::k_Exit;
    send_ipc_msg(&cmd, sizeof(cmd));

    exit(EXIT_SUCCESS);
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

bool CLCommandHandle::get_cmd_line(char *buff, size_t len) {
    assert(len >= 0);

    int ret = m_buffer.read(m_cmd_fd);
    if (ret < 0) {
        utility::unix_error("Receive client command error");
    } else if (ret == 0) {
        utility::debug_info("Client exited");
        close(m_cmd_fd);

        exit(0);
    } else {
        return m_buffer.get_line(buff, len);
//        return m_buffer.get_crlf_line(buff, len);
    }
}

void CLCommandHandle::parse_cmd(char *cmd_line) {
    std::string str_cmd = std::string(cmd_line);
    /* 去除行尾回车换行符 */
    strip_crlf(str_cmd);
    /* 分割命令及参数 */
    split_cmd_and_argv(str_cmd, k_SP_delimiter);
}

void CLCommandHandle::strip_crlf(std::string &str) {
    if (str.size() < 2) {
        return;
    }
    do {
        auto r_pos = str.size() - 1;
        if (str[r_pos] == '\n' || str[r_pos] == '\r') {
            str.erase(r_pos);
        } else {
            break;
        }
    } while (str.size() >= 2);
}

void CLCommandHandle::split_cmd_and_argv(std::string &str, const std::string &delimiter) {
    m_client_cmd_vec.clear();

    size_t pos_0 = 0, pos_1 = 0;
    while ((pos_1 = str.find_first_of(delimiter, pos_0)) != std::string::npos) {
        if (pos_1 == pos_0) {
            pos_0 = pos_1 + 1;
            continue;
        }
        m_client_cmd_vec.push_back(str.substr(pos_0, pos_1 - pos_0));
        pos_0 = pos_1 + 1;
    }
    m_client_cmd_vec.push_back(str.substr(pos_0, pos_1 - pos_0));

    if (!m_client_cmd_vec.empty()) {
        /* 将命令转换为大写 */
        std::string &cmd = m_client_cmd_vec[0];
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), toupper);
    }
}

ipc_utility::EMState CLCommandHandle::do_send_list(bool verbose) {
    std::shared_ptr<tcp::CLConnection> connection = get_data_connection();  /* 数据连接 */

    DIR *dir = opendir(".");
    if (dir == nullptr) /* 目录打开失败 */
        return ipc_utility::Error;

    struct dirent *ent;
    struct stat sbuf{0};

    while ((ent = readdir(dir)) != nullptr) { /* 读取文件夹的信息 */
        if (lstat(ent->d_name, &sbuf) < 0)
            continue;
        if (strncmp(ent->d_name, ".", 1) == 0)
            continue; /* 忽略隐藏的文件 */
        const char *permInfo = utility::get_file_perms(sbuf); /* 获得权限信息 */
        char buf[1024] = {0};
        if (verbose) { /* 是否要发送完整的信息的话 */
            int offset = 0;
            offset += sprintf(buf, "%s ", permInfo);
            offset += sprintf(buf + offset, " %3ld %-8d %-8d ",
                              sbuf.st_nlink, sbuf.st_uid, sbuf.st_gid);
            offset += sprintf(buf + offset, "%8lu ", (unsigned long) sbuf.st_size); /* 文件大小 */
            const char *dateInfo = utility::get_file_date(sbuf); /* 获得文件时间信息 */
            offset += sprintf(buf + offset, "%s ", dateInfo);
            if (S_ISLNK(sbuf.st_mode)) { /* 如果表示的是链接文件的话 */
                char name[1024] = {0};
                readlink(ent->d_name, name, sizeof(name));
                offset += sprintf(buf + offset, "%s -> %s\r\n", ent->d_name, name);
            } else {
                offset += sprintf(buf + offset, "%s\r\n", ent->d_name);
            }
        } else {
            sprintf(buf, "%s\r\n", ent->d_name);
        }

        tcp::send_data(connection->get_fd(), buf, strlen(buf));
    }
    closedir(dir); /* 关闭文件夹 */
    return ipc_utility::Success;
}

std::shared_ptr<tcp::CLConnection> CLCommandHandle::get_data_connection() {
    int data_fd = 0;
    ipc_utility::EMIpcCmd cmd = ipc_utility::k_GetFd;
    ipc_utility::EMState state;
    tcp::send_data(m_pipe_fd, &cmd, sizeof(cmd));
    tcp::recv_data(m_pipe_fd, &state, sizeof(state));

    if (state == ipc_utility::Success) {
        data_fd = ipc_utility::recv_fd(m_pipe_fd);  /* 进程间传递描述符 */
    }
    return std::make_shared<tcp::CLConnection>(data_fd);
}

bool CLCommandHandle::is_file_existed(const char *file_name) {
    struct stat st{0};
    return lstat(file_name, &st) == 0;
}








