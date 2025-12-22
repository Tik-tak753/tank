#include "gameplay/Tank.h"

#include "gameplay/Bullet.h"
#include <QtGlobal>

namespace {
constexpr int kDefaultDeltaMs = 16;
constexpr int kDestructionDelayMs = 350;
}

Tank::Tank(const QPoint& cell)
    : GameObject(QPointF(cell))
    , m_cell(cell)
{
    m_health.setMaxHealth(1);
    m_health.setLives(1);
    m_weapon.setReloadTime(400);
    setSpeed(m_speed);
    syncRenderPositions(true);
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
    m_stepIntervalMs = stepIntervalMsForSpeed(speed);
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
    m_renderPositionPrevious = m_renderPositionCurrent;

    if (m_destroyed) {
        m_destructionTimerMs -= deltaMs;
        return;
    }

    // базовий танк лише відраховує перезарядку
    m_weapon.tick(deltaMs);
}

int Tank::stepIntervalMsForSpeed(float speed) const
{
    if (speed <= 0.0f)
        return 0;

    constexpr int kMillisPerSecond = 1000;
    const float tilesPerSecond = speed;
    const float stepsPerSecond = tilesPerSecond * kStepsPerTile;
    return qMax(1, static_cast<int>(kMillisPerSecond / stepsPerSecond));
}

void Tank::setCell(const QPoint& cell)
{
    m_cell = cell;
    syncRenderPositions(false);
    resetSubTileProgress();
}

void Tank::resetSubTileProgress()
{
    m_subTileProgress = 0;
}

void Tank::syncRenderPositions(bool resetPrevious)
{
    m_renderPositionCurrent = QPointF(m_cell);
    if (resetPrevious)
        m_renderPositionPrevious = m_renderPositionCurrent;
    m_position = m_renderPositionCurrent;
}

QPoint Tank::directionDelta(Direction dir)
{
    switch (dir) {
    case Direction::Up:    return QPoint(0, -1);
    case Direction::Down:  return QPoint(0, 1);
    case Direction::Left:  return QPoint(-1, 0);
    case Direction::Right: return QPoint(1, 0);
    }
    return QPoint(0, 0);
}

void Tank::updateRenderPosition(Direction dir)
{
    const QPointF offset = QPointF(directionDelta(dir)) * (static_cast<qreal>(m_subTileProgress) / kStepsPerTile);
    m_renderPositionCurrent = QPointF(m_cell) + offset;
    m_position = m_renderPositionCurrent;
}

std::unique_ptr<Bullet> Tank::tryShoot()
{
    if (m_destroyed)
        return nullptr;

    if (!m_fireRequested)
        return nullptr;

    m_fireRequested = false;
    return m_weapon.fire(cell(), m_direction, m_type, bulletStepIntervalMs(), bulletCanPierceSteel());
}

int Tank::bulletStepIntervalMs() const
{
    return Bullet::kDefaultStepIntervalMs;
}

bool Tank::bulletCanPierceSteel() const
{
    return false;
}

bool Tank::receiveDamage(int dmg)
{
    m_health.takeDamage(dmg);
    return true;
}
