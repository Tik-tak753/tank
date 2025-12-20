#include "gameplay/EnemyTank.h"

#include <QRandomGenerator>
#include <algorithm>

#include "world/Map.h"
#include "world/Tile.h"

EnemyTank::EnemyTank(const QPoint& cell)
    : Tank(cell)
{
    setDirection(Direction::Down);
    resetFireInterval();
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
    return m_map->isInside(nextCell) && m_map->tile(nextCell).type == TileType::Empty;
}

void EnemyTank::tryMove()
{
    if (!m_map)
        return;

    // Randomly change direction to add variability
    const bool changeDirection = QRandomGenerator::global()->bounded(100) < 25;
    if (changeDirection) {
        setDirection(randomDirection(direction()));
    }

    if (canMove(direction())) {
        setCell(cell() + directionDelta());
        return;
    }

    // When blocked, pick a new random direction (not the current one) and try to move once
    const Direction newDirection = randomDirection(direction());
    setDirection(newDirection);
    if (canMove(newDirection)) {
        setCell(cell() + directionDelta());
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
