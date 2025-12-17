#ifndef WEAPONSYSTEM_H
#define WEAPONSYSTEM_H

#include <QPointF>

#include "gameplay/Direction.h"

class Bullet;

/*
 * WeaponSystem відповідає за перезарядку та створення снарядів.
 */
class WeaponSystem
{
public:
    void setReloadTime(int ms) { m_reloadMs = ms; }
    void setBulletSpeed(float speed) { m_bulletSpeed = speed; }

    void tick(int deltaMs);

    bool canShoot() const;
    Bullet* fire(const QPointF& pos, Direction dir);

private:
    int m_reloadMs = 500;
    int m_cooldownMs = 0;
    float m_bulletSpeed = 4.0f;
};

#endif // WEAPONSYSTEM_H
