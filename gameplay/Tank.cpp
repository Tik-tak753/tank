#include "gameplay/Tank.h"

#include "gameplay/Bullet.h"

Tank::Tank(const QPoint& cell)
    : m_cell(cell)
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
    // базовий танк лише відраховує перезарядку
    m_weapon.tick(16);
}

Bullet* Tank::tryShoot()
{
    if (!m_fireRequested)
        return nullptr;

    m_fireRequested = false;
    const QPointF bulletPos = QPointF(m_cell) + QPointF(0.5, 0.5);
    return m_weapon.fire(bulletPos, m_direction);
}
