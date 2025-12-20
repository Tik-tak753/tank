#include "gameplay/Tank.h"

#include "gameplay/Bullet.h"

namespace {
constexpr int kDefaultDeltaMs = 16;
constexpr int kDestructionDelayMs = 350;
}

Tank::Tank(const QPoint& cell)
    : GameObject(QPointF(cell))
{
    m_health.setMaxHealth(1);
    m_health.setLives(1);
    m_weapon.setReloadTime(400);
}

TankType Tank::getType()
{
    return m_type;
}

void Tank::setType(TankType type)
{
    m_type = type;
}

void Tank::setSpeed(float speed)
{
    m_speed = speed;
}

void Tank::markDestroyed()
{
    if (m_destroyed)
        return;

    m_destroyed = true;
    m_destructionTimerMs = kDestructionDelayMs;
}

void Tank::update()
{
    updateWithDelta(kDefaultDeltaMs);
}

void Tank::updateWithDelta(int deltaMs)
{
    if (m_destroyed) {
        m_destructionTimerMs -= deltaMs;
        return;
    }

    // базовий танк лише відраховує перезарядку
    m_weapon.tick(deltaMs);
}

std::unique_ptr<Bullet> Tank::tryShoot()
{
    if (m_destroyed)
        return nullptr;

    if (!m_fireRequested)
        return nullptr;

    m_fireRequested = false;
    return m_weapon.fire(cell(), m_direction, m_type);
}
