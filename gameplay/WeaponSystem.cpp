#include "gameplay/WeaponSystem.h"

#include <QtGlobal>

#include "gameplay/Bullet.h"

void WeaponSystem::tick(int deltaMs)
{
    if (m_cooldownMs > 0)
        m_cooldownMs = qMax(0, m_cooldownMs - deltaMs);
}

bool WeaponSystem::canShoot() const
{
    return m_cooldownMs == 0;
}

Bullet* WeaponSystem::fire(const QPointF& pos, Direction dir)
{
    if (!canShoot())
        return nullptr;

    m_cooldownMs = m_reloadMs;
    return new Bullet(pos, dir, m_bulletSpeed);
}
