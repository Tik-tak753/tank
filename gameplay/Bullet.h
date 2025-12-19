#ifndef BULLET_H
#define BULLET_H

#include <QPoint>
#include <QtGlobal>

#include "gameplay/Direction.h"

/*
 * Bullet відповідає за рух снаряду та його життєвий цикл.
 */
class Bullet
{
public:
    Bullet(const QPoint& cell, Direction dir);

    QPoint cell() const { return m_cell; }
    Direction direction() const { return m_direction; }
    QPoint directionDelta() const;
    QPoint nextCell() const;
    bool isAlive() const { return m_alive; }

    void update(int deltaMs);
    void destroy();

private:
    QPoint m_cell;
    Direction m_direction;
    qsizetype m_elapsedMs = 0;
    bool m_alive = true;
};

#endif // BULLET_H
