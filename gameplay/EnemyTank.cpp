#include "gameplay/EnemyTank.h"

#include <QRandomGenerator>

EnemyTank::EnemyTank(const QPoint& cell)
    : Tank(cell)
{
    setDirection(Direction::Down);
    resetFireInterval();
}

void EnemyTank::update()
{
    updateWithDelta(16);
}

void EnemyTank::updateWithDelta(int deltaMs)
{
    Tank::updateWithDelta(deltaMs);

    m_elapsedMs += deltaMs;
    if (m_elapsedMs >= m_fireIntervalMs) {
        requestFire();
        m_elapsedMs = 0;
        resetFireInterval();
    }
}

void EnemyTank::resetFireInterval()
{
    // Стріляємо з випадковою затримкою в межах 800–1200 мс
    m_fireIntervalMs = QRandomGenerator::global()->bounded(800, 1201);
}
