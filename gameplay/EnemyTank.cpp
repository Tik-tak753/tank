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
    updateWithDelta(16);
}

void EnemyTank::updateWithDelta(int deltaMs)
{
    Tank::updateWithDelta(deltaMs);
    if (m_ai)
        m_ai->tick(*this);
}
