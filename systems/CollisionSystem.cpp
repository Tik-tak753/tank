#include "systems/CollisionSystem.h"

#include "core/GameState.h"
#include "gameplay/Bullet.h"
#include "gameplay/Tank.h"
#include "world/Base.h"
#include "world/Map.h"

void CollisionSystem::resolve(Map& map, QList<Tank*>& tanks, QList<Bullet*>& bullets, Base* base, GameState& state)
{
    for (qsizetype i = 0; i < bullets.size(); ++i) {
        Bullet* bullet = bullets.at(i);
        if (!bullet || !bullet->isAlive())
            continue;

        const QPoint cell = bullet->cell();
        bool destroyBullet = false;

        if (!map.isInside(cell))
            destroyBullet = true;

        for (Tank* tank : tanks) {
            if (tank && tank->cell() == cell) {
                tank->health().takeDamage(1);
                destroyBullet = true;
                break;
            }
        }

        if (base && base->cell() == cell) {
            base->takeDamage();
            state.setBaseDestroyed();
            destroyBullet = true;
        }

        if (destroyBullet)
            bullet->destroy();
    }

}
