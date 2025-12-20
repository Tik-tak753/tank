#include "systems/CollisionSystem.h"

#include <QtGlobal>

#include "core/GameState.h"
#include "gameplay/Bullet.h"
#include "gameplay/EnemyTank.h"
#include "gameplay/PlayerTank.h"
#include "gameplay/Tank.h"
#include "world/Base.h"
#include "world/Map.h"
#include "world/Tile.h"
#include "enums/enums.h"

void CollisionSystem::resolve(
    Map& map,
    QList<Tank*>& tanks,
    QList<Bullet*>& bullets,
    Base* base,
    GameState& state)
{
    for (qsizetype i = 0; i < bullets.size(); ++i) {
        Bullet* bullet = bullets.at(i);
        if (!bullet || !bullet->isAlive())
            continue;

        const QPoint cell = bullet->cell();
        bool destroyBullet = false;
        bool spawnBulletExplosion = true;

        destroyBullet = handleBulletMapCollision(*bullet, map, base, state, spawnBulletExplosion);

        // ---- Tank collision ----
        if (!destroyBullet) {
            for (Tank* tank : tanks) {
                if (!tank)
                    continue;

                if (tank->isDestroyed())
                    continue;

                auto owner = bullet->type();

                if (tank->getType() == owner)
                    continue;

                if (owner) {
                    const bool ownerIsPlayer = owner == TankType::Player;
                    const bool ownerIsEnemy = owner == TankType::Enemy;
                    const bool targetIsPlayer = tank->getType() == TankType::Player;
                    const bool targetIsEnemy = tank->getType() == TankType::Enemy;

                    if ((ownerIsPlayer && targetIsPlayer) || (ownerIsEnemy && targetIsEnemy))
                        continue;
                }

                if (tank->cell() == cell) {
                    tank->health().takeDamage(1);

                    if (auto enemy = dynamic_cast<EnemyTank*>(tank)) {
                        if (enemy->health().isAlive())
                            enemy->triggerHitFeedback();
                    }

                    destroyBullet = true;
                    spawnBulletExplosion = false;
                    break;
                }
            }
        }

        if (destroyBullet)
            bullet->destroy(spawnBulletExplosion);
    }
}

bool CollisionSystem::handleBulletMapCollision(
    Bullet& bullet,
    Map& map,
    Base* base,
    GameState& state,
    bool& spawnBulletExplosion)
{
    const QPoint cell = bullet.cell();

    if (!map.isInside(cell)) {
        return true;
    }

    Tile tile = map.tile(cell);

    switch (tile.type) {
    case TileType::Empty:
        return false;

    case TileType::Brick: {
        tile.takeDamage(1);
        if (tile.isDestroyed()) {
            map.setTile(cell, TileFactory::empty());
        } else {
            map.setTile(cell, tile);
        }
        return true;
    }

    case TileType::Steel: {
        // Сталеві плитки не руйнуються базовими снарядами.
        if (tile.destructible) {
            tile.takeDamage(1);
            if (tile.isDestroyed())
                map.setTile(cell, TileFactory::empty());
            else
                map.setTile(cell, tile);
        }

        return true;
    }

    case TileType::Base: {
        if (base) {
            base->takeDamage();

            if (base->isDestroyed() && !state.isBaseDestroyed()) {
                state.setBaseDestroyed();
                map.setTile(cell, TileFactory::empty());
            }
        }

        return true;
    }
    }

    Q_UNREACHABLE();
}
