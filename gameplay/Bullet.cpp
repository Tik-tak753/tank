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
      m_canPierceSteel(canPierceSteel),
      m_renderPositionCurrent(QPointF(cell)),
      m_renderPositionPrevious(QPointF(cell))
{
    m_subStepIntervalMs = (kStepsPerTile > 0) ? static_cast<qreal>(m_stepIntervalMs) / static_cast<qreal>(kStepsPerTile) : 0.0;
}

void Bullet::update(int deltaMs)
{
    if (!m_alive)
        return;

    m_renderPositionPrevious = m_renderPositionCurrent;

    m_elapsedMs += static_cast<qreal>(deltaMs);
    if (m_subStepIntervalMs <= 0.0) {
        updateRenderPosition();
        return;
    }

    while (m_elapsedMs >= m_subStepIntervalMs) {
        m_elapsedMs -= m_subStepIntervalMs;
        ++m_subTileProgress;

        if (m_subTileProgress >= kStepsPerTile) {
            m_subTileProgress = 0;
            m_cell += stepDelta(m_direction);
        }

        updateRenderPosition();
    }

    updateRenderPosition();
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

QPointF Bullet::visualTilePosition() const
{
    if (kStepsPerTile <= 0)
        return QPointF(m_cell);

    const qreal progress = static_cast<qreal>(m_subTileProgress) / static_cast<qreal>(kStepsPerTile);
    return QPointF(m_cell) + QPointF(directionDelta()) * progress;
}

void Bullet::updateRenderPosition()
{
    m_renderPositionCurrent = visualTilePosition();
}
