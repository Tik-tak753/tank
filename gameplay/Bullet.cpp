#include "gameplay/Bullet.h"

namespace {
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

Bullet::Bullet(const QPoint& cell, Direction dir, const TankType type, int stepIntervalMs, bool canPierceSteel)
    : m_cell(cell),
      m_direction(dir),
      m_ownerType(type),
      m_stepIntervalMs(stepIntervalMs),
      m_canPierceSteel(canPierceSteel)
{
}

void Bullet::update(int deltaMs)
{
    if (!m_alive)
        return;

    m_elapsedMs += static_cast<qsizetype>(deltaMs);
    if (m_elapsedMs < m_stepIntervalMs)
        return;

    m_elapsedMs -= m_stepIntervalMs;
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

void Bullet::destroy(bool spawnExplosion)
{
    m_spawnExplosionOnDestroy = spawnExplosion;
    m_alive = false;
}
