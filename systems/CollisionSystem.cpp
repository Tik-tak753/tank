#include "systems/CollisionSystem.h"

#include <QtGlobal>

#include "core/GameState.h"
#include "gameplay/Bullet.h"
#include "gameplay/Tank.h"
#include "world/Base.h"
#include "world/Map.h"
#include "world/Tile.h"

void CollisionSystem::resolve(Map& map, QList<Tank*>& tanks, QList<Bullet*>& bullets, Base* base, GameState& state)
{
    for (qsizetype i = 0; i < bullets.size(); ++i) {
        Bullet* bullet = bullets.at(i);
        if (!bullet || !bullet->isAlive())
            continue;

        const QPoint nextCell = bullet->nextCell();
        bool destroyBullet = false;

        if (!map.isInside(nextCell)) {
            destroyBullet = true;
        } else {
            const Tile target = map.tile(nextCell);
            switch (target.type) {
            case TileType::Empty:
                break;
            case TileType::Brick:
                map.setTile(nextCell, TileFactory::empty());
                destroyBullet = true;
                break;
            case TileType::Steel:
                destroyBullet = true;
                break;
            case TileType::Base:
                destroyBullet = true;
                state.setBaseDestroyed();
                if (base)
                    base->takeDamage();
                break;
            }
        }

        if (!destroyBullet) {
            for (Tank* tank : tanks) {
                if (tank && tank->cell() == nextCell) {
                    tank->health().takeDamage(1);
                    destroyBullet = true;
                    break;
                }
            }
        }

        if (destroyBullet)
            bullet->destroy();
    }

}
