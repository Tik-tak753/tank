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

            switch (target.type) {
            case TileType::Empty:
                break;

            case TileType::Brick:
                map.setTile(cell, TileFactory::empty());
                destroyBullet = true;
                break;

            case TileType::Steel:
                qDebug() << "[BULLET] Hit STEEL tile at"
                         << QStringLiteral("(%1, %2)").arg(cell.x()).arg(cell.y())
                         << "→ blocked";
                destroyBullet = true;
                break;

            case TileType::Base:
                destroyBullet = true;

                if (base && !state.isBaseDestroyed()) {
                    base->takeDamage();

                    if (base->isDestroyed()) {
                        state.setBaseDestroyed();
                        map.setTile(cell, TileFactory::empty());
                    }
                }
                break;
            }
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
