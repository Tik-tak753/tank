#include "gameplay/HealthSystem.h"

void HealthSystem::takeDamage(int dmg)
{
    m_health -= dmg;
    if (m_health <= 0 && m_lives > 0) {
        --m_lives;
        if (m_lives > 0)
            m_health = m_maxHealth;
    }
}

void HealthSystem::restoreFullHealth()
{
    m_health = m_maxHealth;
}
