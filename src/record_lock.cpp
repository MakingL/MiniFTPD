//
// Created by MLee on 2019/12/30.
//
#include "record_lock.h"

namespace record_lock {
    int lock_reg(int fd, int cmd, short int type, size_t offset, short int whence, size_t len) { /* 文件锁 */
        struct flock lock{0};
        lock.l_type = type;
        lock.l_start = offset;
        lock.l_whence = whence;
        lock.l_len = len;
        return (fcntl(fd, cmd, &lock));
    }
}