#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <QList>
#include <QPoint>

class Map;
class Tank;
class Bullet;
class Base;
class GameState;

/*
 * CollisionSystem визначає взаємодію снарядів із плитками та танками.
 */
class CollisionSystem
{
public:
    void resolve(Map& map, QList<Tank*>& tanks, QList<Bullet*>& bullets, Base* base, GameState& state);
};

#endif // COLLISIONSYSTEM_H
