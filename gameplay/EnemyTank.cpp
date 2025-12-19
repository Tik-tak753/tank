#include "gameplay/EnemyTank.h"

#include <QRandomGenerator>

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

void EnemyTank::tryMove()
{
    if (!m_map)
        return;

    const QPoint nextCell = cell() + directionDelta();
    if (m_map->isInside(nextCell) && m_map->tile(nextCell).type == TileType::Empty) {
        setCell(nextCell);
    } else {
        setDirection(oppositeDirection());
    }
}

void EnemyTank::resetFireInterval()
{
    // Стріляємо з випадковою затримкою в межах 800–1200 мс
    m_fireIntervalMs = QRandomGenerator::global()->bounded(800, 1201);
}
