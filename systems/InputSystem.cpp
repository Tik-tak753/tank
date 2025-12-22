#include "systems/InputSystem.h"

#include <algorithm>
#include <Qt>

std::optional<Direction> InputSystem::directionFromKey(int key)
{
    switch (key) {
    case Qt::Key_W:
    case Qt::Key_Up:
        return Direction::Up;
    case Qt::Key_S:
    case Qt::Key_Down:
        return Direction::Down;
    case Qt::Key_A:
    case Qt::Key_Left:
        return Direction::Left;
    case Qt::Key_D:
    case Qt::Key_Right:
        return Direction::Right;
    default:
        return std::nullopt;
    }
}

bool InputSystem::handleKeyPress(int key)
{
    if (key == Qt::Key_Space) {
        requestFire();
        return true;
    }

    const std::optional<Direction> dir = directionFromKey(key);
    if (!dir.has_value())
        return false;

    pushDirection(*dir);
    return true;
}

bool InputSystem::handleKeyRelease(int key)
{
    if (key == Qt::Key_Space)
        return true;

    const std::optional<Direction> dir = directionFromKey(key);
    if (!dir.has_value())
        return false;

    removeDirection(*dir);
    return true;
}

std::optional<Direction> InputSystem::currentDirection() const
{
    if (m_pressedDirections.empty())
        return std::nullopt;

    return m_pressedDirections.back();
}

void InputSystem::requestFire()
{
    m_fireRequested = true;
}

bool InputSystem::consumeFire()
{
    if (!m_fireRequested)
        return false;
    m_fireRequested = false;
    return true;
}

void InputSystem::clear()
{
    m_pressedDirections.clear();
    m_fireRequested = false;
}

void InputSystem::pushDirection(Direction dir)
{
    const auto it = std::find(m_pressedDirections.begin(), m_pressedDirections.end(), dir);
    if (it != m_pressedDirections.end()) {
        m_pressedDirections.erase(it);
    }
    m_pressedDirections.push_back(dir);
}

void InputSystem::removeDirection(Direction dir)
{
    const auto it = std::find(m_pressedDirections.begin(), m_pressedDirections.end(), dir);
    if (it != m_pressedDirections.end()) {
        m_pressedDirections.erase(it);
    }
}
