#include "gameplay/Bullet.h"

namespace {
constexpr qsizetype kStepIntervalMs = 120;

QPoint stepDelta(Direction dir)
{
    switch (dir) {
    case Direction::Up:    return QPoint(0, -1);
    case Direction::Down:  return QPoint(0, 1);
    case Direction::Left:  return QPoint(-1, 0);
    case Direction::Right: return QPoint(1, 0);
    }

    return QPoint(0, 0);
}
} // namespace

Bullet::Bullet(const QPoint& cell, Direction dir)
    : m_cell(cell),
      m_direction(dir)
{
}

void Bullet::update(int deltaMs)
{
    if (!m_alive)
        return;

    m_elapsedMs += static_cast<qsizetype>(deltaMs);
    if (m_elapsedMs < kStepIntervalMs)
        return;

    m_elapsedMs -= kStepIntervalMs;
    m_cell += stepDelta(m_direction);
}

QPoint Bullet::directionDelta() const
{
    return stepDelta(m_direction);
}

QPoint Bullet::nextCell() const
{
    return m_cell + directionDelta();
}

void Bullet::destroy()
{
    m_alive = false;
}
