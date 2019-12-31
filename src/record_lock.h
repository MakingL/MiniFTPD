//
// Created by MLee on 2019/12/30.
//

#ifndef MINIFTPD_RECORD_LOCK_H
#define MINIFTPD_RECORD_LOCK_H

#include "common.h"

namespace record_lock {
    int lock_reg(int fd, int cmd, short int type, size_t offset, short int whence, size_t len);

    inline int write_w_lock(int fd, size_t offset, short int whence, size_t len) {
        return lock_reg(fd, F_SETLKW, F_WRLCK, offset, whence, len);
    }

    inline int read_w_lock(int fd, size_t offset, short int whence, size_t len) {
        return lock_reg(fd, F_SETLKW, F_RDLCK, offset, whence, len);
    }

    inline int unlock(int fd, size_t offset, short int whence, size_t len) {
        return lock_reg(fd, F_SETLK, F_UNLCK, offset, whence, len);
    }
}

/* 文件记录读锁 */
class CLRDRecordLock {
public:
    explicit CLRDRecordLock(int fd) : m_fd(fd) {
        record_lock::write_w_lock(m_fd, SEEK_SET, 0, 0);
    }

    ~CLRDRecordLock() {
        record_lock::unlock(m_fd, SEEK_SET, 0, 0);
    }

private:
    int m_fd;
};

class CLWRRecordLock {
public:
    explicit CLWRRecordLock(int fd) : m_fd(fd) {
        record_lock::read_w_lock(m_fd, SEEK_SET, 0, 0);
    }

    ~CLWRRecordLock() {
        record_lock::unlock(m_fd, SEEK_SET, 0, 0);
    }

private:
    int m_fd;
};
#endif //MINIFTPD_RECORD_LOCK_H
