#include "systems/CollisionSystem.h"

#include "core/GameState.h"
#include "gameplay/Bullet.h"
#include "gameplay/Tank.h"
#include "world/Base.h"
#include "world/Map.h"

void CollisionSystem::resolve(Map& map, QList<Tank*>& tanks, QList<Bullet*>& bullets, Base* base, GameState& state)
{
    Q_UNUSED(state);

    for (int i = bullets.size() - 1; i >= 0; --i) {
        Bullet* bullet = bullets.at(i);
        const QPoint cell = bullet->cell();

        bool removeBullet = false;

        if (!map.isInside(cell))
            removeBullet = true;

        for (Tank* tank : tanks) {
            if (tank->cell() == cell) {
                tank->health().takeDamage(1);
                removeBullet = true;
                break;
            }
        }

        if (base && base->cell() == cell)
            removeBullet = true;

        if (removeBullet) {
            delete bullet;
            bullets.removeAt(i);
        }
    }

    for (int i = tanks.size() - 1; i >= 0; --i) {
        Tank* tank = tanks.at(i);
        if (!tank->health().isAlive()) {
            delete tank;
            tanks.removeAt(i);
        }
    }
}
