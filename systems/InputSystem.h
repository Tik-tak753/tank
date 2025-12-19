#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include "gameplay/Direction.h"
#include <optional>
#include <vector>

/*
 * InputSystem зберігає останній стан вводу користувача.
 * Подальша інтеграція з подіями Qt дозволить оновлювати цей стан.
 */
class InputSystem
{
public:
    bool handleKeyPress(int key);
    bool handleKeyRelease(int key);

    std::optional<Direction> currentDirection() const;

    void requestFire();
    bool consumeFire();

private:
    static std::optional<Direction> directionFromKey(int key);
    void pushDirection(Direction dir);
    void removeDirection(Direction dir);

    std::vector<Direction> m_pressedDirections;
    bool m_fireRequested = false;
};

#endif // INPUTSYSTEM_H
