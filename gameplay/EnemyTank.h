#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include "gameplay/Tank.h"

/*
 * EnemyTank — примітивний супротивник, який стріляє за таймером.
 */
class EnemyTank : public Tank
{
public:
    explicit EnemyTank(const QPoint& cell);

    void update() override;
    void updateWithDelta(int deltaMs) override;

private:
    void resetFireInterval();

    int m_elapsedMs = 0;
    int m_fireIntervalMs = 1000;
};

#endif // ENEMYTANK_H
