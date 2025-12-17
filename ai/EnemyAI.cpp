#include "ai/EnemyAI.h"

#include "gameplay/EnemyTank.h"

void EnemyAI::tick(EnemyTank& tank)
{
    m_movement.tick(tank);
    m_shooting.tick(tank);
}
