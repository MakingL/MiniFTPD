//
// Created by MLee on 2019/12/22.
//
#include "common.h"
#include "utility.h"

namespace utility {
    const char* get_file_perms(struct stat& sbuf) { /* 获得文件的权限 */
        static char perms[] = "----------";
        perms[0] = '?';

        mode_t mode = sbuf.st_mode;
        switch (mode & S_IFMT) {
            case S_IFREG:
                perms[0] = '-';
                break;
            case S_IFLNK:
                perms[0] = 'l'; /* 链接文件 */
                break;
            case S_IFDIR:
                perms[0] = 'd'; /* 目录文件 */
                break;
            case S_IFIFO:
                perms[0] = 'p'; /* 管道文件 */
                break;
            case S_IFCHR:
                perms[0] = 'c';
                break;
            case S_IFSOCK:
                perms[0] = 's'; /* socket文件 */
                break;
            case S_IFBLK:
                perms[0] = 'b'; /* 块设备 */
        }

        if (mode & S_IRUSR) { /* 获得权限值 */
            perms[1] = 'r';
        }

        if (mode & S_IWUSR) {
            perms[2] = 'w';
        }

        if (mode & S_IXUSR) {
            perms[3] = 'r';
        }

        if (mode & S_IWGRP) {
            perms[4] = 'r';
        }

        if (mode & S_IWGRP) {
            perms[5] = 'w';
        }

        if (mode & S_IXGRP) {
            perms[6] = 'x';
        }

        if (mode & S_IROTH) {
            perms[7] = 'r';
        }

        if (mode & S_IWOTH) {
            perms[8] = 'w';
        }

        if (mode & S_IXOTH) {
            perms[9] = 'x';
        }

        if (mode & S_ISUID) {
            perms[3] = (perms[3] == 'x') ? 's' : 'S';
        }

        if (mode & S_ISGID) {
            perms[6] = (perms[6] == 'x') ? 's' : 'S';
        }

        if (mode & S_ISVTX) {
            perms[9] = (perms[9] == 'x') ? 't' : 'T';
        }

        return perms;
    }

    const char* get_file_date(struct stat& sbuf) {
        static char datebuf[64] = { 0 };
        const char* format = "%b %e %H:%M";
        struct timeval tv{0};
        gettimeofday(&tv, nullptr);
        time_t localTime = tv.tv_sec;
        if (sbuf.st_mtime > localTime || (localTime - sbuf.st_mtime) > 60*60*24*182) {
            format = "%b %e %Y";
        }
        struct tm* p = localtime(&sbuf.st_mtime);
        strftime(datebuf, sizeof(datebuf), format, p); /* 将格式处理成字符串 */
        return datebuf;
    }
}