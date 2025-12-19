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

        const QPoint cell = bullet->cell();
        bool destroyBullet = false;

        if (!map.isInside(cell)) {
            destroyBullet = true;
        } else {
            const bool hitBase = base && base->cell() == cell;
            if (hitBase) {
                destroyBullet = true;
                if (!state.isBaseDestroyed()) {
                    if (!base->isDestroyed())
                        base->takeDamage();

                    if (base->isDestroyed()) {
                        state.setBaseDestroyed();
                        map.setTile(cell, TileFactory::empty());
                    }
                }
            } else {
                const Tile target = map.tile(cell);
                switch (target.type) {
                case TileType::Empty:
                    break;
                case TileType::Brick:
                    map.setTile(cell, TileFactory::empty());
                    destroyBullet = true;
                    break;
                case TileType::Steel:
                    destroyBullet = true;
                    break;
                case TileType::Base:
                    destroyBullet = true;
                    if (base && !state.isBaseDestroyed()) {
                        if (!base->isDestroyed())
                            base->takeDamage();

                        if (base->isDestroyed()) {
                            state.setBaseDestroyed();
                            map.setTile(cell, TileFactory::empty());
                        }
                    }
                    break;
                }
            }
        }

        if (!destroyBullet) {
            for (Tank* tank : tanks) {
                if (!tank)
                    continue;

                if (tank == bullet->owner())
                    continue;

                if (tank->cell() == cell) {
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
