#ifndef ENEMYAI_H
#define ENEMYAI_H

#include "ai/MovementController.h"
#include "ai/ShootingController.h"

class EnemyTank;

/*
 * EnemyAI агрегує декілька контролерів для ухвалення простих рішень.
 */
class EnemyAI
{
public:
    void tick(EnemyTank& tank);

private:
    MovementController m_movement;
    ShootingController m_shooting;
};

#endif // ENEMYAI_H
