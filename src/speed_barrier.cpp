//
// Created by MLee on 2020/1/8.
//

#include "common.h"
#include "speed_barrier.h"
#include "utility.h"

void CLSpeedBarrier::limit_speed(size_t transferred_size) {
    auto time_now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = time_now - m_start_time;
    int64_t elapsed_micro_seconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    double elapsed_seconds = elapsed.count();

    auto speed_now = static_cast<int64_t >(transferred_size / elapsed_seconds);
    if (speed_now > m_max_speed) {
        int64_t speed_diff = speed_now - m_max_speed;
        int64_t need_sleep_micro_seconds = (speed_diff / static_cast<double >(m_max_speed)) * elapsed_micro_seconds;

        /* 睡眠 */
        struct timespec timeSpan{0};
        timeSpan.tv_sec = need_sleep_micro_seconds / k_micro_sec_per_sec;
        // 1微秒等于1000纳秒
        timeSpan.tv_nsec = (need_sleep_micro_seconds - timeSpan.tv_sec * k_micro_sec_per_sec) * 1000; /* 纳秒级别的精度 */

        int res;
        do {
            res = nanosleep(&timeSpan, &timeSpan);
        } while (res == -1 && errno == EINTR); /* 可能被信号中断,所以要用do while格式 */
    }

    m_start_time = std::chrono::system_clock::now();    /* 重新计时 */
}
