//
// Created by MLee on 2019/12/27.
#include "buffer.h"

#include "common.h"

CLBuffer::CLBuffer(size_t initial_size)
        : m_buffer(initial_size + k_cheap_prepend),
          m_reader_index(0),
          m_writer_index(0) {

}

size_t CLBuffer::read(int fd) {
    char extra_buff[65536];

    /* struct iovec 为 IO 向量  */
    int writable_size = writable_bytes();
    struct iovec vec[2];
    vec[0].iov_base = writable_begin();
    vec[0].iov_len = writable_size;
    vec[1].iov_base = extra_buff;
    vec[1].iov_len = sizeof(extra_buff);

    const int iov_cnt = (writable_bytes() < sizeof(extra_buff)) ? 2 : 1;
    /* readv writev 在一个原子操作中读取或写入多个缓冲区  */
    const ssize_t return_size = readv(fd, vec, iov_cnt);
    if (return_size <= 0) {
        return return_size;
    } else if (static_cast<size_t>(return_size) <= writable_size) {
        on_wrote_data_size(return_size);
    } else {
        /* 可写的部分空间已经写满 */
        on_wrote_data_size(writable_size);
        /* 将额外读出的部分数据写入到缓冲区 */
        append_data(extra_buff, return_size - writable_size);
    }
    return return_size;
}

bool CLBuffer::get_line(char *dest, size_t len, bool clear_overflowed) {
    const char *p_line_end = find_EOL();
    if (!p_line_end) {
        /* 还没有找到一行数据 */
        if (clear_overflowed && readable_bytes() >= len) {
            on_have_read(readable_bytes());
        }
        return false;
    }

    const char *p_data_start = peek();
    assert(p_data_start <= p_line_end);
    ptrdiff_t data_size = p_line_end - p_data_start + 1;
    if (len < static_cast<size_t >(data_size)) {
        if (clear_overflowed) {
            on_have_read(data_size);
        }
        // return false;
        /* 此处返回空字符串，以给客户端反馈提示信息 */
        dest[0] = '\0';
        return true;
    }

    std::copy(p_data_start, p_line_end + 1, dest);
    dest[data_size] = '\0';
    on_have_read(data_size);

    return true;
}

bool CLBuffer::get_crlf_line(char *dest, size_t len, bool clear_overflowed) {
    const char *p_line_end = find_CRLF();
    if (!p_line_end) {
        /* 还没有找到一行数据 */
        if (clear_overflowed && readable_bytes() >= len) {
            on_have_read(readable_bytes());
        }
        return false;
    }
    /* 指向最后一个字符*/
    p_line_end = p_line_end + 1;

    const char *p_data_start = peek();
    assert(p_data_start <= p_line_end);
    ptrdiff_t data_size = p_line_end - p_data_start + 1;
    if (len < static_cast<size_t >(data_size)) {
        if (clear_overflowed) {
            on_have_read(data_size);
        }
        // return false;
        /* 此处返回空字符串，以给客户端反馈提示信息 */
        dest[0] = '\0';
        return true;
    }

    std::copy(p_data_start, p_line_end + 1, dest);
    dest[data_size] = '\0';
    on_have_read(data_size);

    return true;
}

void CLBuffer::adjust_space_for(size_t len) {
    if (writable_bytes() + recyclable_bytes() < len) {
        /* 调整的 Buffer 大小直接定为 2 的整数次幂，防止过于频繁的空间扩容 */
        size_t need_size = m_writer_index + len;
        size_t size_new = pow(2, static_cast<int >(log2(need_size)) + 1) + k_cheap_prepend;
        m_buffer.resize(size_new);
    } else { /* 充分利用目前已分配的空间后，能够容纳该大小的空间 */
        assert(0 < m_reader_index);
        size_t readable_size = readable_bytes();
        std::copy(begin() + m_reader_index, begin() + m_writer_index, begin());
        m_reader_index = 0;
        m_writer_index = m_reader_index + readable_size;
        assert(readable_size == readable_bytes());
    }
}


