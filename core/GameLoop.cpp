#include "core/GameLoop.h"

GameLoop::GameLoop(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(16);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        if (m_tick)
            m_tick();
    });
}

void GameLoop::setTickCallback(const std::function<void()>& callback)
{
    m_tick = callback;
}

void GameLoop::setInterval(int ms)
{
    m_timer.setInterval(ms);
}

void GameLoop::start()
{
    m_timer.start();
}

void GameLoop::stop()
{
    m_timer.stop();
}
