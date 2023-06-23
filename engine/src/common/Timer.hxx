#pragma once

#include <chrono>

class cTimer
{
public:
    void start();
    void stop();

    bool isStarted() const;

    /**
     * @brief delta time between start and current.
     * @return time in microseconds.
     */
    uint32_t getCurrentDelta() const;

    /**
     * @brief delta time between start and stop.
     * @return time in microseconds.
     */
    uint32_t getDelta() const;

    /**
     * @brief System time.
     * @return time in microseconds.
     */
    static uint32_t getCurrentTime();

private:
    using Clock = std::chrono::high_resolution_clock;
    using MSec = std::chrono::milliseconds;
    using Us = std::chrono::microseconds;

    using TimePoint = Clock::time_point;
    uint32_t getDelta(const TimePoint& begin, const TimePoint& end) const;

private:
    bool m_isStarted = false;

    TimePoint m_start;
    TimePoint m_stop;
};