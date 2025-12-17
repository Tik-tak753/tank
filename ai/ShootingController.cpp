#include "ai/ShootingController.h"

#include "gameplay/EnemyTank.h"

void ShootingController::tick(EnemyTank& tank)
{
    if (m_cooldownMs > 0) {
        m_cooldownMs -= 16;
        return;
    }

    m_cooldownMs = m_reloadMs;
    tank.requestFire();
}
