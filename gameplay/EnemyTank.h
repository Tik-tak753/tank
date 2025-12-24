#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include <QColor>
#include "gameplay/Tank.h"

class Map;

struct EnemyStats
{
    int stepsPerTile = 0;
    int stepIntervalMs = 0;
    int armorHits = 1;
    bool dropsBonus = false;
    int fireCooldownMs = 1000;
    QColor baseColor;
    QColor damagedColor;
};

/*
 * EnemyTank — примітивний супротивник, який стріляє за таймером.
 */
class EnemyTank : public Tank
{
public:
    EnemyTank(const QPoint& cell, EnemyType type = EnemyType::Basic);

    void setMap(const Map* map) { m_map = map; }
    void setFrozen(bool frozen) { m_frozen = frozen; }
    bool isFrozen() const { return m_frozen; }

    // Простий AI: рухається та стріляє за таймерами
    void update() override;
    // Обробка руху з урахуванням заморозки та перезарядки
    void updateWithDelta(int deltaMs) override;
    // Рахує попадання й перемикає ефект броні
    bool receiveDamage(int dmg) override;

    bool isHitFeedbackActive() const { return m_hitFeedbackTimerMs > 0; }
    void triggerHitFeedback();
    EnemyType enemyType() const { return m_enemyType; }
    const EnemyStats& stats() const { return m_stats; }
    int armorHitsRemaining() const { return health().health(); }
    int maxArmorHits() const { return m_stats.armorHits; }
    bool dropsBonus() const { return m_stats.dropsBonus; }
    QColor currentColor() const;

private:
    QPoint directionDelta() const;
    QPoint directionDelta(Direction direction) const;
    Direction oppositeDirection() const;
    Direction randomDirection(Direction exclude) const;
    bool canMove(Direction direction) const;
    bool shouldSlide() const;
    void tryMove();
    void resetFireInterval();
    void applyStats();
    static const EnemyStats& statsForType(EnemyType type);
    static float tilesPerSecondFromStats(const EnemyStats& stats);

    const Map* m_map = nullptr;
    EnemyType m_enemyType = EnemyType::Basic;
    EnemyStats m_stats;

    int m_fireElapsedMs = 0;
    int m_fireIntervalMs = 1000;

    int m_moveElapsedMs = 0;
    int m_moveIntervalMs = 600;

    int m_hitFeedbackTimerMs = 0;
    bool m_sliding = false;
    bool m_frozen = false;
};

#endif // ENEMYTANK_H
