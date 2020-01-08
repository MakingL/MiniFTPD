//
// Created by MLee on 2020/1/8.
//

#ifndef MINIFTPD_SPEED_BARRIER_H
#define MINIFTPD_SPEED_BARRIER_H

#include <chrono>

class CLSpeedBarrier {
public:
    explicit CLSpeedBarrier(int64_t max_speed) : m_max_speed(max_speed) {
        m_start_time = std::chrono::system_clock::now();
    }

    void limit_speed(size_t transferred_size);

private:
    static const int k_micro_sec_per_sec = 1000 * 1000; /* 1秒等于1000000微秒 */

private:
    int64_t m_max_speed;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
};

#endif //MINIFTPD_SPEED_BARRIER_H
