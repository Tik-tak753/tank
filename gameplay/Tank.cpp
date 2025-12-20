#include "gameplay/Tank.h"

#include <QDebug>

#include "gameplay/Bullet.h"

namespace {
constexpr int kDefaultDeltaMs = 16;
constexpr int kDestructionDelayMs = 350;
}

#ifdef QT_DEBUG
QSet<const Tank*> Tank::s_aliveTanks;
#endif

Tank::Tank(const QPoint& cell)
    : GameObject(QPointF(cell))
{
#ifdef QT_DEBUG
    Q_ASSERT(!s_aliveTanks.contains(this));
    s_aliveTanks.insert(this);
#endif
    m_health.setMaxHealth(1);
    m_health.setLives(1);
    m_weapon.setReloadTime(400);
}

Tank::~Tank()
{
#ifdef QT_DEBUG
    Q_ASSERT(s_aliveTanks.contains(this));
    s_aliveTanks.remove(this);
    qDebug() << "Tank destroyed:" << this;
#endif
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
    return m_weapon.fire(cell(), m_direction, this);
}
