//
// Created by MLee on 2019/12/27.
//

#ifndef MINIFTPD_BUFFER_H
#define MINIFTPD_BUFFER_H

#include <cstddef>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <cassert>

class CLBuffer {
public:
    static const size_t k_initial_size = 1024;
    /* 增加一点额外空间，避免频繁调整空间 */
    static const size_t k_cheap_prepend = 8;

    explicit CLBuffer(size_t initial_size = k_initial_size);

    size_t readable_bytes() const {
        return m_writer_index - m_reader_index;
    }

    size_t writable_bytes() const {
        return m_buffer.size() - m_writer_index;
    }

    void append_data(const char *data, size_t len) {
        ensure_safe_write(len);
        std::copy(data, data + len, writable_begin());
        on_wrote_data_size(len);
    }

    size_t read(int fd);

    bool get_line(char *dest, size_t len, bool clear_overflowed = true);

    bool get_crlf_line(char *dest, size_t len, bool clear_overflowed = true);

private:
    const char *begin() const {
        return &(*m_buffer.begin());
    }

    char *begin() {
        return &(*m_buffer.begin());
    }

    char *writable_begin() {
        return begin() + m_writer_index;
    }

    const char *writable_begin() const {
        return begin() + m_writer_index;
    }

    size_t recyclable_bytes() const {
        return m_reader_index;
    }

    const char *peek() const {
        return begin() + m_reader_index;
    }

    const char *find_EOL() const {
        return static_cast<const char *>(memchr(peek(), '\n', readable_bytes()));
    }

    const char *find_CRLF() const {
        const char *p_crlf = std::search(peek(), writable_begin(), k_CRLF, k_CRLF + 2);
        return p_crlf == writable_begin() ? nullptr : p_crlf;
    }


    void on_have_read(size_t len) { /* 调整读取后的位置 */
        assert(len <= readable_bytes());
        if (len < readable_bytes()) {
            m_reader_index += len;
        } else if (len == readable_bytes()) {
            on_have_read_all();
        }
    }

    void on_have_read_all() {
        /* 将 Buffer 的写 index 退回到开始 */
        m_reader_index = 0;
        m_writer_index = 0;
        m_buffer[m_reader_index] = m_buffer[m_reader_index + 1] = '\0';
    }

    void ensure_safe_write(size_t len) {
        /* 保证可写入的空间足够 */
        if (writable_bytes() < len) {
            /* 调整空间 */
            adjust_space_for(len);
        }
        assert(writable_bytes() >= len);
    }

    void on_wrote_data_size(size_t data_size) {
        m_writer_index += data_size;
    }

    void adjust_space_for(size_t len);

private:
    std::vector<char> m_buffer;
    size_t m_reader_index;
    size_t m_writer_index;

    const char *k_CRLF = "\r\n";
};

#endif //MINIFTPD_BUFFER_H
