#include "gameplay/Bullet.h"

#include <QDebug>

#ifdef QT_DEBUG
QSet<const Bullet*> Bullet::s_aliveBullets;
#endif

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

Bullet::Bullet(const QPoint& cell, Direction dir, const Tank* owner)
    : m_cell(cell),
      m_direction(dir),
      m_owner(owner)
{
#ifdef QT_DEBUG
    Q_ASSERT(!s_aliveBullets.contains(this));
    s_aliveBullets.insert(this);
#endif
}

Bullet::~Bullet()
{
#ifdef QT_DEBUG
    Q_ASSERT(s_aliveBullets.contains(this));
    s_aliveBullets.remove(this);
    qDebug() << "Bullet destroyed:" << this;
#endif
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

void Bullet::destroy(bool spawnExplosion)
{
    m_spawnExplosionOnDestroy = spawnExplosion;
    m_alive = false;
}
