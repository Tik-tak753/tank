#ifndef TANK_H
#define TANK_H

#include <QPoint>
#include <QPointF>
#include <memory>

#include "gameplay/Direction.h"
#include "gameplay/GameObject.h"
#include "gameplay/WeaponSystem.h"
#include "gameplay/HealthSystem.h"

#include "enums/enums.h"

class Bullet;

/*
 * Tank — базовий клас, що описує спільні властивості танків
 * (позиція, напрямок, здоров'я, зброя).
 */
class Tank : public GameObject
{
private:
    TankType m_type;

public:

    explicit Tank(const QPoint& cell);
    virtual ~Tank() = default;

    TankType getType();

    void setType(TankType type);

    bool isDestroyed() const { return m_destroyed; }
    bool isDestructionFinished() const { return m_destroyed && m_destructionTimerMs <= 0; }
    void markDestroyed();

    QPoint cell() const { return m_cell; }
    QPointF renderPosition() const { return m_renderPositionCurrent; }
    QPointF previousRenderPosition() const { return m_renderPositionPrevious; }
    void setCell(const QPoint& cell);

    Direction direction() const { return m_direction; }
    void setDirection(Direction dir) { m_direction = dir; }

    float speed() const { return m_speed; }
    void setSpeed(float speed);

    HealthSystem& health() { return m_health; }
    const HealthSystem& health() const { return m_health; }
    WeaponSystem& weapon() { return m_weapon; }

    void requestFire() { m_fireRequested = true; }
    virtual bool receiveDamage(int dmg);

    virtual void update();
    virtual void updateWithDelta(int deltaMs);
    virtual std::unique_ptr<Bullet> tryShoot();
    virtual int bulletStepIntervalMs() const;
    virtual bool bulletCanPierceSteel() const;

protected:
    static constexpr int kStepsPerTile = 16;
    static constexpr float kDefaultTilesPerSecond = 3.9f;

    int stepIntervalMsForSpeed(float speed) const;
    int stepIntervalMs() const { return m_stepIntervalMs; }
    bool isAlignedToGrid() const { return m_subTileProgress == 0; }
    void resetSubTileProgress();
    void updateRenderPosition(Direction dir);
    void syncRenderPositions(bool resetPrevious = true);

    static QPoint directionDelta(Direction dir);

    Direction m_direction = Direction::Up;
    float m_speed = kDefaultTilesPerSecond;

    QPoint m_cell;
    QPointF m_renderPositionCurrent;
    QPointF m_renderPositionPrevious;

    int m_stepIntervalMs = 0;
    int m_stepAccumulatorMs = 0;
    int m_subTileProgress = 0;

    bool m_fireRequested = false;

    WeaponSystem m_weapon;
    HealthSystem m_health;

    bool m_destroyed = false;
    int m_destructionTimerMs = 0;
}; 

#endif // TANK_H
