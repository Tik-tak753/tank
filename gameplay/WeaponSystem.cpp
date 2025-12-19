#include "gameplay/WeaponSystem.h"

#include "gameplay/Bullet.h"

namespace {
QPoint directionDelta(Direction dir)
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

void WeaponSystem::tick(int deltaMs)
{
    if (m_cooldownMs > 0)
        m_cooldownMs = qMax(0, m_cooldownMs - deltaMs);
}

bool WeaponSystem::canShoot() const
{
    return m_cooldownMs == 0;
}

std::unique_ptr<Bullet> WeaponSystem::fire(const QPoint& cell, Direction dir)
{
    if (!canShoot())
        return nullptr;

    m_cooldownMs = m_reloadMs;
    return std::make_unique<Bullet>(cell + directionDelta(dir), dir);
}
