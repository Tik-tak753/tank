#include "systems/CollisionSystem.h"

#include <QDebug>
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

namespace {
TileCollisionResult resolveTileCollision(const TileType type)
{
    TileCollisionResult result;

    switch (type) {
    case TileType::Empty:
        break;

    case TileType::Brick:
        qDebug() << "[COLLISION] Bullet ↔ Brick → tile destroyed";
        result.destroyBullet = true;
        result.destroyTile = true;
        break;

    case TileType::Steel:
        qDebug() << "[COLLISION] Bullet ↔ Steel → bullet destroyed, tile intact";
        result.destroyBullet = true;
        break;

    case TileType::Base:
        qDebug() << "[COLLISION] Bullet ↔ Base → base damaged";
        result.destroyBullet = true;
        result.damageBase = true;
        break;
    }

    return result;
}
} // namespace

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

        // ---- Out of bounds ----
        if (!map.isInside(cell)) {
            destroyBullet = true;
        } else {
            const Tile target = map.tile(cell);
            TileCollisionResult tileResult = resolveTileCollision(target.type);

            if (tileResult.destroyTile)
                map.setTile(cell, TileFactory::empty());

            if (tileResult.damageBase && base && !state.isBaseDestroyed()) {
                base->takeDamage();

                if (base->isDestroyed()) {
                    state.setBaseDestroyed();
                    map.setTile(cell, TileFactory::empty());
                }
            }

            if (tileResult.destroyBullet)
                destroyBullet = true;
        }

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
