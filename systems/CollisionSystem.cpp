#include "systems/CollisionSystem.h"

#include <QDebug>
#include <QString>
#include <QStringList>
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
    switch (type) {
    case TileType::Empty:
        return {};

    case TileType::Brick:
        return {true, true, false};

    case TileType::Steel:
        return {true, false, false};

    case TileType::Base:
        return {true, false, true};
    }

    Q_UNREACHABLE();
}

QString tileTypeToString(const TileType type)
{
    switch (type) {
    case TileType::Empty:
        return "Empty";
    case TileType::Brick:
        return "Brick";
    case TileType::Steel:
        return "Steel";
    case TileType::Base:
        return "Base";
    }

    Q_UNREACHABLE();
}

QString formatTileCollisionLog(const TileType type, const TileCollisionResult& result)
{
    QStringList effects;

    if (result.destroyBullet)
        effects << "bullet destroyed";

    if (result.destroyTile)
        effects << "tile destroyed";

    if (result.damageBase)
        effects << "base damaged";

    const QString effectText = effects.isEmpty() ? "no effect" : effects.join(", ");

    return QString("[COLLISION] Bullet ↔ %1 → %2").arg(tileTypeToString(type), effectText);
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
            const TileCollisionResult tileResult = resolveTileCollision(target.type);

            qDebug() << formatTileCollisionLog(target.type, tileResult);

            if (tileResult.destroyTile)
                map.setTile(cell, TileFactory::empty());

            if (tileResult.damageBase && base) {
                base->takeDamage();

                if (base->isDestroyed() && !state.isBaseDestroyed()) {
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
