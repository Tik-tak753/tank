#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include "gameplay/Tank.h"

class Map;

/*
 * EnemyTank — примітивний супротивник, який стріляє за таймером.
 */
class EnemyTank : public Tank
{
public:
    explicit EnemyTank(const QPoint& cell);

    void setMap(const Map* map) { m_map = map; }

    void update() override;
    void updateWithDelta(int deltaMs) override;

private:
    QPoint directionDelta() const;
    Direction oppositeDirection() const;
    void tryMove();
    void resetFireInterval();

    const Map* m_map = nullptr;

    int m_fireElapsedMs = 0;
    int m_fireIntervalMs = 1000;

    int m_moveElapsedMs = 0;
    int m_moveIntervalMs = 600;
};

#endif // ENEMYTANK_H
