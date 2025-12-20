#include "gameplay/EnemyTank.h"

#include <QRandomGenerator>
#include <QVector>
#include <algorithm>

#include "world/Map.h"
#include "world/Tile.h"

EnemyTank::EnemyTank(const QPoint& cell)
    : Tank(cell)
{
    setDirection(Direction::Down);
    resetFireInterval();
    setType(TankType::Enemy);
}

void EnemyTank::update()
{
    updateWithDelta(16);
}

void EnemyTank::updateWithDelta(int deltaMs)
{
    Tank::updateWithDelta(deltaMs);

    if (m_hitFeedbackTimerMs > 0)
        m_hitFeedbackTimerMs = std::max(0, m_hitFeedbackTimerMs - deltaMs);

    if (isDestroyed())
        return;

    m_moveElapsedMs += deltaMs;
    if (m_moveElapsedMs >= m_moveIntervalMs) {
        m_moveElapsedMs = 0;
        tryMove();
    }

    m_fireElapsedMs += deltaMs;
    if (m_fireElapsedMs >= m_fireIntervalMs) {
        requestFire();
        m_fireElapsedMs = 0;
        resetFireInterval();
    }
}

QPoint EnemyTank::directionDelta() const
{
    switch (direction()) {
    case Direction::Up:    return QPoint(0, -1);
    case Direction::Down:  return QPoint(0, 1);
    case Direction::Left:  return QPoint(-1, 0);
    case Direction::Right: return QPoint(1, 0);
    }
    return QPoint(0, 0);
}

QPoint EnemyTank::directionDelta(Direction direction) const
{
    switch (direction) {
    case Direction::Up:    return QPoint(0, -1);
    case Direction::Down:  return QPoint(0, 1);
    case Direction::Left:  return QPoint(-1, 0);
    case Direction::Right: return QPoint(1, 0);
    }
    return QPoint(0, 0);
}

Direction EnemyTank::oppositeDirection() const
{
    switch (direction()) {
    case Direction::Up:    return Direction::Down;
    case Direction::Down:  return Direction::Up;
    case Direction::Left:  return Direction::Right;
    case Direction::Right: return Direction::Left;
    }
    return Direction::Down;
}

Direction EnemyTank::randomDirection(Direction exclude) const
{
    Direction newDirection = direction();
    do {
        const int value = QRandomGenerator::global()->bounded(4);
        switch (value) {
        case 0: newDirection = Direction::Up; break;
        case 1: newDirection = Direction::Down; break;
        case 2: newDirection = Direction::Left; break;
        case 3: newDirection = Direction::Right; break;
        }
    } while (newDirection == exclude);

    return newDirection;
}

bool EnemyTank::canMove(Direction direction) const
{
    if (!m_map)
        return false;

    const QPoint nextCell = cell() + directionDelta(direction);
    if (!m_map->isInside(nextCell))
        return false;

    return m_map->isWalkable(nextCell);
}

void EnemyTank::tryMove()
{
    if (!m_map)
        return;

    QVector<Direction> availableDirections;
    availableDirections.reserve(4);
    for (Direction dir : {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
        if (canMove(dir))
            availableDirections.append(dir);
    }

    if (availableDirections.isEmpty())
        return;

    Direction current = direction();
    const bool blocked = !canMove(current);
    if (blocked || QRandomGenerator::global()->bounded(100) < 25) {
        QVector<Direction> candidates = availableDirections;

        // уникаємо розворотів на 180 градусів, якщо є інші опції
        const Direction opposite = oppositeDirection();
        if (candidates.size() > 1)
            candidates.removeOne(opposite);

        if (!candidates.isEmpty())
            current = candidates.at(QRandomGenerator::global()->bounded(candidates.size()));
        else
            current = opposite;
        setDirection(current);
    }

    if (canMove(current)) {
        const QPoint next = cell() + directionDelta(current);
        setDirection(current);
        setCell(next);
    }
}

void EnemyTank::resetFireInterval()
{
    // Стріляємо з випадковою затримкою в межах 800–1200 мс
    m_fireIntervalMs = QRandomGenerator::global()->bounded(800, 1201);
}

void EnemyTank::triggerHitFeedback()
{
    constexpr int kHitFeedbackDurationMs = 120;
    m_hitFeedbackTimerMs = kHitFeedbackDurationMs;
}
