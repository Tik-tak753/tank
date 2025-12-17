#ifndef BULLET_H
#define BULLET_H

#include <QPointF>

#include "gameplay/Direction.h"

/*
 * Bullet відповідає за рух снаряду та його життєвий цикл.
 */
class Bullet
{
public:
    Bullet(const QPointF& pos, Direction dir, float speed);

    QPointF position() const { return m_position; }
    Direction direction() const { return m_direction; }

    void update(int deltaMs);
    bool isExpired() const;

private:
    QPointF m_position;
    Direction m_direction;
    float m_speed;
    int m_lifetimeMs = 1200;
    int m_elapsedMs = 0;
};

#endif // BULLET_H
