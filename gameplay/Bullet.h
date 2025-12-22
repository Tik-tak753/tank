#ifndef BULLET_H
#define BULLET_H

#include <QPoint>
#include <QtGlobal>

#include "gameplay/Direction.h"
#include "enums/enums.h"
class Tank;

/*
 * Bullet відповідає за рух снаряду та його життєвий цикл.
 */
class Bullet
{
public:
    // ~130 мс на плитку — снаряд пролітає приблизно дві плитки, поки базовий танк минає одну.
    static constexpr int kDefaultStepIntervalMs = 130;

    Bullet(const QPoint& cell, Direction dir, const TankType type, int stepIntervalMs = kDefaultStepIntervalMs, bool canPierceSteel = false);

    QPoint cell() const { return m_cell; }
    Direction direction() const { return m_direction; }
    QPoint directionDelta() const;
    QPoint nextCell() const;
    bool isAlive() const { return m_alive; }
    TankType type() const { return m_ownerType; }
    bool canPierceSteel() const { return m_canPierceSteel; }

    void update(int deltaMs);
    void destroy(bool spawnExplosion = true);
    bool spawnExplosionOnDestroy() const { return m_spawnExplosionOnDestroy; }

private:
    QPoint m_cell;
    Direction m_direction;
    //const Tank* m_owner = nullptr;
    const TankType m_ownerType;
    qsizetype m_elapsedMs = 0;
    int m_stepIntervalMs = kDefaultStepIntervalMs;
    bool m_alive = true;
    bool m_spawnExplosionOnDestroy = true;
    bool m_canPierceSteel = false;
};

#endif // BULLET_H
