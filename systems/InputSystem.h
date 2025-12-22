#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include "gameplay/Direction.h"
#include <optional>
#include <vector>
#include <QtGlobal>

/*
 * InputSystem зберігає останній стан вводу користувача.
 * Подальша інтеграція з подіями Qt дозволить оновлювати цей стан.
 */
class InputSystem
{
public:
    bool handleKeyPress(int key);
    bool handleKeyRelease(int key);
    bool handleKeyPress(int key, quint32 scanCode);
    bool handleKeyRelease(int key, quint32 scanCode);

    std::optional<Direction> currentDirection() const;

    void requestFire();
    bool consumeFire();
    void clear();

private:
    static std::optional<Direction> directionFromKey(int key);
    void pushDirection(Direction dir);
    void removeDirection(Direction dir);
    static std::optional<Direction> directionFromScanCode(quint32 scanCode);

    std::vector<Direction> m_pressedDirections;
    bool m_fireRequested = false;
};

#endif // INPUTSYSTEM_H
