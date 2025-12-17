#include "world/LevelLoader.h"

#include "world/Tile.h"
#include "world/Wall.h"

LevelData LevelLoader::loadDefaultLevel(const GameRules& rules) const
{
    LevelData data;
    const QSize mapSize = rules.mapSize();
    data.map = std::make_unique<Map>(mapSize.width(), mapSize.height());

    data.playerSpawn = QPoint(mapSize.width() / 2, mapSize.height() - 2);
    data.enemySpawns = {
        QPoint(1, 1),
        QPoint(mapSize.width() - 2, 1),
        QPoint(mapSize.width() / 2, 1)
    };
    data.baseCell = rules.baseCell();

    // зовнішня рамка сталі
    for (int x = 0; x < mapSize.width(); ++x) {
        data.map->setTile(QPoint(x, 0), TileFactory::steel());
        data.map->setTile(QPoint(x, mapSize.height() - 1), TileFactory::steel());
    }
    for (int y = 0; y < mapSize.height(); ++y) {
        data.map->setTile(QPoint(0, y), TileFactory::steel());
        data.map->setTile(QPoint(mapSize.width() - 1, y), TileFactory::steel());
    }

    // орел та захист цеглою
    data.map->setTile(data.baseCell, TileFactory::base());
    QList<QPoint> shield = {
        data.baseCell + QPoint(1, 0), data.baseCell + QPoint(-1, 0),
        data.baseCell + QPoint(0, -1), data.baseCell + QPoint(1, -1),
        data.baseCell + QPoint(-1, -1)
    };
    for (const QPoint& p : shield)
        data.map->setTile(p, TileFactory::brick());

    // коридори для руху ворогів
    for (int y = 2; y < mapSize.height() - 2; y += 4) {
        for (int x = 2; x < mapSize.width() - 2; ++x)
            data.map->setTile(QPoint(x, y), TileFactory::brick());
    }

    return data;
}
