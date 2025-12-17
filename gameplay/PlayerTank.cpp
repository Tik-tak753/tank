#include "gameplay/PlayerTank.h"

#include "systems/InputSystem.h"

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

void PlayerTank::update()
{
    Tank::update();

    if (!m_input)
        return;

    const Direction desired = m_input->currentDirection();
    setDirection(desired);

    QPoint delta(0, 0);
    switch (desired) {
    case Direction::Up:    delta.setY(-1); break;
    case Direction::Down:  delta.setY(1);  break;
    case Direction::Left:  delta.setX(-1); break;
    case Direction::Right: delta.setX(1);  break;
    }
    setCell(cell() + delta);

    if (m_input->consumeFire())
        requestFire();
}
