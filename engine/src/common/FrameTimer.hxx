#pragma once

#include "Timer.hxx"

class cFrameTimer
{
public:
    void restart();
    void update();

    float getFrameDeltaTime() const;
    float getFps() const;

private:
    cTimer m_timer;

    uint32_t m_lastTime;
    uint32_t m_frame;

    float m_frameDelta;
    float m_countingTime;
    float m_fps;
};
