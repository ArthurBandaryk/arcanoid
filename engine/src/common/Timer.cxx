#include "Timer.hxx"

void cTimer::start()
{
    m_isStarted = true;
    m_start = Clock::now();
}

void cTimer::stop()
{
    m_isStarted = false;
    m_stop = Clock::now();
}

bool cTimer::isStarted() const
{
    return m_isStarted;
}

uint32_t cTimer::getCurrentDelta() const
{
    auto currentTime = Clock::now();
    return getDelta(m_start, currentTime);
}

uint32_t cTimer::getDelta() const
{
    return getDelta(m_start, m_stop);
}

uint32_t cTimer::getDelta(const TimePoint& begin, const TimePoint& end) const
{
    auto deltaTime = std::chrono::duration_cast<Us>(end - begin);
    return static_cast<uint32_t>(deltaTime.count());
}

uint32_t cTimer::getCurrentTime()
{
    auto currentTime = std::chrono::duration_cast<Us>(Clock::now().time_since_epoch());
    return static_cast<uint32_t>(currentTime.count());
}