#ifndef TANK_H
#define TANK_H

#include <QPoint>
#include <memory>

#include "gameplay/Direction.h"
#include "gameplay/GameObject.h"
#include "gameplay/WeaponSystem.h"
#include "gameplay/HealthSystem.h"

class Bullet;

/*
 * Tank — базовий клас, що описує спільні властивості танків
 * (позиція, напрямок, здоров'я, зброя).
 */
class Tank : public GameObject
{
public:
    explicit Tank(const QPoint& cell);
    virtual ~Tank() = default;

    QPoint cell() const { return GameObject::cell(); }
    void setCell(const QPoint& cell) { GameObject::setCell(cell); }

    Direction direction() const { return m_direction; }
    void setDirection(Direction dir) { m_direction = dir; }

    float speed() const { return m_speed; }
    void setSpeed(float speed);

    HealthSystem& health() { return m_health; }
    WeaponSystem& weapon() { return m_weapon; }

    void requestFire() { m_fireRequested = true; }

    virtual void update();
    virtual void updateWithDelta(int deltaMs);
    virtual std::unique_ptr<Bullet> tryShoot();

protected:
    Direction m_direction = Direction::Up;
    float m_speed = 1.0f;

    bool m_fireRequested = false;

    WeaponSystem m_weapon;
    HealthSystem m_health;
};

#endif // TANK_H
