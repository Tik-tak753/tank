#include "gameplay/PlayerTank.h"

#include "systems/InputSystem.h"
#include "world/Map.h"
#include <optional>

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
}

PlayerTank::PlayerTank(const QPoint& cell)
    : Tank(cell)
{
    m_health.setLives(3);
    m_speed = 1.5f;
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

    if (!m_input)
        return;

    const std::optional<Direction> desired = m_input->currentDirection();
    if (!desired.has_value())
        return;

    setDirection(*desired);

    // Підтримуємо стабільний рух по тайлах навіть за різного FPS
    const float deltaSeconds = deltaMs / 1000.0f;
    m_moveAccumulator += speed() * deltaSeconds;

    const QPoint step = directionDelta(*desired);
    while (m_moveAccumulator >= 1.0f) {
        const QPoint nextCell = cell() + step;
        if (m_map && !m_map->isWalkable(nextCell)) {
            m_moveAccumulator = 0.0f; // блокуємо накопичення в глухих кутах
            break;
        }

        setCell(nextCell);
        m_moveAccumulator -= 1.0f;
    }

    if (m_input->consumeFire())
        requestFire();
}
