#include "gameplay/EnemyTank.h"

#include "ai/EnemyAI.h"

EnemyTank::EnemyTank(const QPoint& cell)
    : Tank(cell)
{
    m_speed = 1.0f;
    m_ai = std::make_unique<EnemyAI>();
}

EnemyTank::~EnemyTank() = default;

void EnemyTank::update()
{
    Tank::update();
    if (m_ai)
        m_ai->tick(*this);
}
