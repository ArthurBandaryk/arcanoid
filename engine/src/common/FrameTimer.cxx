#include "FrameTimer.hxx"

void cFrameTimer::restart()
{
    m_timer.start();
    m_lastTime = m_timer.getCurrentTime();
    m_frame = 0;
    m_frameDelta = 0.0f;
    m_countingTime = 0.0f;
    m_fps = 0.0f;
}

void cFrameTimer::update()
{
    const uint32_t currentTime = m_timer.getCurrentTime();
    const float dt = (currentTime - m_lastTime) * 0.000001f;
    m_lastTime = currentTime;

    m_frame++;
    m_frameDelta = dt;

    m_countingTime += dt;
    if (m_countingTime >= 1.0f)
    {
        m_fps = m_frame / m_countingTime;
        m_countingTime = 0.0f;
        m_frame = 0;
    }
}

float cFrameTimer::getFrameDeltaTime() const
{
    return m_frameDelta;
}

float cFrameTimer::getFps() const
{
    return m_fps;
}