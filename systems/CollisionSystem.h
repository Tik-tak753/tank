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
 * CollisionSystem координує перевірки, але не містить правил поведінки тайлів.
 * Вона читає маски блокування з Tile і вирішує, чи може куля/танк пройти,
 * залишаючи саму «політику» тайлів у даних карти.
 */
class CollisionSystem
{
public:
    void resolve(Map& map, QList<Tank*>& tanks, QList<Bullet*>& bullets, Base* base, GameState& state);

private:
    bool handleBulletMapCollision(Bullet& bullet, Map& map, Base* base, GameState& state, bool& spawnBulletExplosion);
};

#endif // COLLISIONSYSTEM_H
