#include "gameplay/PlayerTank.h"

#include "systems/InputSystem.h"
#include "world/Map.h"
#include <optional>
#include "world/Tile.h"

namespace {
QPoint directionDelta(Direction dir)
{
    switch (dir) {
    case Direction::Up:    return QPoint(0, -1);
    case Direction::Down:  return QPoint(0, 1);
    case Direction::Left:  return QPoint(-1, 0);
    case Direction::Right: return QPoint(1, 0);
    }
    return QPoint(0, 0);
}

bool shouldSlideOnIce(const Map* map, const QPoint& cell, Direction direction)
{
    if (!map)
        return false;

    const Tile tile = map->tile(cell);
    if (tile.type != TileType::Ice)
        return false;

    const QPoint nextCell = cell + directionDelta(direction);
    return map->isWalkable(nextCell);
}
}

PlayerTank::PlayerTank(const QPoint& cell)
    : Tank(cell)
{
    m_health.setLives(3);
    m_speed = 1.5f;
    setType(TankType::Player);
    applyUpgrades();
}

void PlayerTank::addStar()
{
    if (m_stars >= kMaxStars)
        return;

    ++m_stars;
    applyUpgrades();
}

bool PlayerTank::canPierceSteel() const
{
    return m_stars >= kMaxStars;
}

void PlayerTank::setInput(InputSystem* input)
{
    m_input = input;
}

void PlayerTank::setMap(const Map* map)
{
    m_map = map;
}

void PlayerTank::update()
{
    updateWithDelta(16);
}

void PlayerTank::updateWithDelta(int deltaMs)
{
    Tank::updateWithDelta(deltaMs);

    if (isDestroyed())
        return;

    if (!m_input)
        return;

    const std::optional<Direction> desired = m_input->currentDirection();
    if (!m_sliding && !desired.has_value())
        return;

    if (!m_sliding && desired.has_value())
        setDirection(*desired);

    // Підтримуємо стабільний рух по тайлах навіть за різного FPS
    const float deltaSeconds = deltaMs / 1000.0f;
    m_moveAccumulator += speed() * deltaSeconds;

    const QPoint step = directionDelta(direction());
    while (m_moveAccumulator >= 1.0f) {
        const QPoint nextCell = cell() + step;
        if (m_map && !m_map->isWalkable(nextCell)) {
            // Прохідність вирішують самі тайли через маску BlockTank:
            // танк не знає деталей карти і рухається, доки дані дозволяють.
            m_moveAccumulator = 0.0f; // блокуємо накопичення в глухих кутах
            m_sliding = false;
            break;
        }

        setCell(nextCell);
        m_moveAccumulator -= 1.0f;

        m_sliding = shouldSlideOnIce(m_map, cell(), direction());
    }

    if (m_input->consumeFire())
        requestFire();
}

int PlayerTank::bulletStepIntervalMs() const
{
    if (m_stars >= 1)
        return 80;

    return Tank::bulletStepIntervalMs();
}

bool PlayerTank::bulletCanPierceSteel() const
{
    return canPierceSteel();
}

void PlayerTank::applyUpgrades()
{
    m_weapon.setReloadTime(reloadTimeMs());
}

int PlayerTank::reloadTimeMs() const
{
    if (m_stars >= 2)
        return 250;

    return 400;
}
