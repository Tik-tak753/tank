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
    void setFrozen(bool frozen) { m_frozen = frozen; }
    bool isFrozen() const { return m_frozen; }

    void update() override;
    void updateWithDelta(int deltaMs) override;

    bool isHitFeedbackActive() const { return m_hitFeedbackTimerMs > 0; }
    void triggerHitFeedback();

private:
    QPoint directionDelta() const;
    QPoint directionDelta(Direction direction) const;
    Direction oppositeDirection() const;
    Direction randomDirection(Direction exclude) const;
    bool canMove(Direction direction) const;
    bool shouldSlide() const;
    void tryMove();
    void resetFireInterval();

    const Map* m_map = nullptr;

    int m_fireElapsedMs = 0;
    int m_fireIntervalMs = 1000;

    int m_moveElapsedMs = 0;
    int m_moveIntervalMs = 600;

    int m_hitFeedbackTimerMs = 0;
    bool m_sliding = false;
    bool m_frozen = false;
};

#endif // ENEMYTANK_H
