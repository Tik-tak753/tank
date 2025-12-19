#include "gameplay/Tank.h"

#include "gameplay/Bullet.h"

namespace {
constexpr int kDefaultDeltaMs = 16;
}

Tank::Tank(const QPoint& cell)
    : GameObject(QPointF(cell))
{
    m_health.setMaxHealth(1);
    m_health.setLives(1);
    m_weapon.setReloadTime(400);
}

void Tank::setSpeed(float speed)
{
    m_speed = speed;
}

void Tank::update()
{
    updateWithDelta(kDefaultDeltaMs);
}

void Tank::updateWithDelta(int deltaMs)
{
    // базовий танк лише відраховує перезарядку
    m_weapon.tick(deltaMs);
}

std::unique_ptr<Bullet> Tank::tryShoot()
{
    if (!m_fireRequested)
        return nullptr;

    m_fireRequested = false;
    return m_weapon.fire(cell(), m_direction);
}
