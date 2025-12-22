#include "gameplay/WeaponSystem.h"

#include "gameplay/Bullet.h"
#include "gameplay/Tank.h"

void WeaponSystem::tick(int deltaMs)
{
    if (m_cooldownMs > 0)
        m_cooldownMs = qMax(0, m_cooldownMs - deltaMs);
}

bool WeaponSystem::canShoot() const
{
    return m_cooldownMs == 0;
}

std::unique_ptr<Bullet> WeaponSystem::fire(const QPoint& cell, Direction dir, const TankType owner, int bulletStepIntervalMs, bool canPierceSteel)
{
    if (!canShoot())
        return nullptr;

    m_cooldownMs = m_reloadMs;
    return std::make_unique<Bullet>(cell + Tank::directionDelta(dir), dir, owner, bulletStepIntervalMs, canPierceSteel);
}
