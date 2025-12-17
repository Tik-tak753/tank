#include "systems/InputSystem.h"

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
