#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include "gameplay/Direction.h"

/*
 * InputSystem зберігає останній стан вводу користувача.
 * Подальша інтеграція з подіями Qt дозволить оновлювати цей стан.
 */
class InputSystem
{
public:
    void setDirection(Direction dir) { m_direction = dir; }
    Direction currentDirection() const { return m_direction; }

    void requestFire();
    bool consumeFire();

private:
    Direction m_direction = Direction::Up;
    bool m_fireRequested = false;
};

#endif // INPUTSYSTEM_H
