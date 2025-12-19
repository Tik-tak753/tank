#include "gameplay/Bullet.h"

#include "world/Map.h"
#include "world/Tile.h"

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

Bullet::Bullet(const QPoint& cell, Direction dir)
    : m_cell(cell),
      m_direction(dir)
{
}

void Bullet::update(int deltaMs, const Map& map)
{
    if (!m_alive)
        return;

    m_elapsedMs += static_cast<qsizetype>(deltaMs);
    if (m_elapsedMs < kStepIntervalMs)
        return;

    const QPoint delta = stepDelta(m_direction);
    while (m_alive && m_elapsedMs >= kStepIntervalMs) {
        const QPoint nextCell = m_cell + delta;
        if (!map.isInside(nextCell)) {
            destroy();
            break;
        }

        const Tile target = map.tile(nextCell);
        if (target.type != TileType::Empty) {
            destroy();
            break;
        }

        m_cell = nextCell;
        m_elapsedMs -= kStepIntervalMs;
    }
}

void Bullet::destroy()
{
    m_alive = false;
}
