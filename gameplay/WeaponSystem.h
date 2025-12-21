#ifndef WEAPONSYSTEM_H
#define WEAPONSYSTEM_H

#include <QPoint>
#include <memory>

#include "gameplay/Direction.h"
#include "enums/enums.h"

class Bullet;
class Tank;

/*
 * WeaponSystem відповідає за перезарядку та створення снарядів.
 */
class WeaponSystem
{
public:
    void setReloadTime(int ms) { m_reloadMs = ms; }

    void tick(int deltaMs);

    bool canShoot() const;
    std::unique_ptr<Bullet> fire(const QPoint& cell, Direction dir, const TankType owner, int bulletStepIntervalMs, bool canPierceSteel);

private:
    int m_reloadMs = 500;
    int m_cooldownMs = 0;
};

#endif // WEAPONSYSTEM_H
