//
// Created by MLee on 2019/12/24.
//

#ifndef MINIFTPD_COMMAND_HANDLE_H
#define MINIFTPD_COMMAND_HANDLE_H

#include <unordered_map>
#include <vector>
#include "ftp_codes.h"

class CLCommandHandle {
public:
    CLCommandHandle(int command_fd, int read_pipe_fd);

    ~CLCommandHandle();

    void handle();

private:
    void reply_client(const char *format, ...);
    void welcome_client();

    bool get_cmd_line(char *buf, size_t len);

    void parse_cmd(char *cmd_line);
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
    void do_rnfy(); /* 重命名开始 */
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
    const char *kCRLF = "\r\n";
//    typedef void (CLCommandHandle::*PCommandExecutor)();
    using PCommandExecutor = void (CLCommandHandle::*)();

    std::unordered_map<std::string, PCommandExecutor> m_cmd_exec_map;
    int m_cmd_fd;
    int m_pipe_fd;

    bool m_b_stop;
    std::vector<std::string> m_client_cmd_vec;
    char client_cmd_line[1024];
};

#endif //MINIFTPD_COMMAND_HANDLE_H