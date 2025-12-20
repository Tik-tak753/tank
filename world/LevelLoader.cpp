#include "world/LevelLoader.h"

#include <algorithm>
#include <array>

#include "world/Tile.h"
#include "world/Wall.h"

namespace {
QPoint defaultPlayerSpawn(const QSize& size, const QPoint& baseCell)
{
    const QPoint leftOfBase(baseCell.x() - 1, baseCell.y());
    if (leftOfBase.x() >= 0 && leftOfBase.x() < size.width() &&
        leftOfBase.y() >= 0 && leftOfBase.y() < size.height()) {
        return leftOfBase;
    }

    return QPoint(std::clamp(size.width() / 2, 0, size.width() - 1),
                  std::clamp(size.height() - 2, 0, size.height() - 1));
}
} // namespace

LevelData LevelLoader::loadFromText(const QStringList& lines, const GameRules& rules) const
{
    LevelData data;
    const bool hasLines = !lines.isEmpty();
    const qsizetype height = hasLines ? lines.size() : static_cast<qsizetype>(rules.mapSize().height());
    qsizetype width = hasLines ? 0 : static_cast<qsizetype>(rules.mapSize().width());

    for (const QString& row : lines)
        width = std::max(width, row.size());

    const QSize mapSize(static_cast<int>(width), static_cast<int>(height));
    data.map = std::make_unique<Map>(mapSize);
    data.baseCell = rules.baseCell();
    data.playerSpawn = defaultPlayerSpawn(mapSize, data.baseCell);
    bool hasPlayerSpawn = false;

    // Легенда символів:
    // '.' або пробіл — порожня клітинка
    // '#' або 'B' — цегляна стіна
    // 'S' або 'X' — сталева стіна
    // 'A' або 'H' — база гравця
    // 'P' — позиція спавну гравця
    // 'E' — позиція спавну ворога
    for (qsizetype y = 0; y < lines.size(); ++y) {
        const QString& row = lines.at(y);
        for (qsizetype x = 0; x < row.size(); ++x) {
            const QChar symbol = row.at(x);
            const QPoint cell(static_cast<int>(x), static_cast<int>(y));

            switch (symbol.toLatin1()) {
            case '#':
            case 'B':
                data.map->setTile(cell, TileFactory::brick());
                break;
            case 'S':
            case 'X':
                data.map->setTile(cell, TileFactory::steel());
                break;
            case 'A': // орел/база
            case 'H':
                data.baseCell = cell;
                data.map->setTile(cell, TileFactory::base());
                break;
            case 'P':
                data.playerSpawn = cell;
                hasPlayerSpawn = true;
                break;
            case 'E':
                data.enemySpawns.append(cell);
                break;
            default:
                data.map->setTile(cell, TileFactory::empty());
                break;
            }
        }
    }

    // гарантуємо наявність бази навіть якщо символу немає в тексті
    if (data.map->isInside(data.baseCell))
        data.map->setTile(data.baseCell, TileFactory::base());

    if (!hasPlayerSpawn)
        data.playerSpawn = defaultPlayerSpawn(mapSize, data.baseCell);

    const QPoint leftOfBase(data.baseCell.x() - 1, data.baseCell.y());
    const QPoint bottomCenter(std::clamp(mapSize.width() / 2, 0, mapSize.width() - 1),
                              std::clamp(mapSize.height() - 2, 0, mapSize.height() - 1));

    const std::array<QPoint, 3> spawnCandidates = {
        data.playerSpawn,
        leftOfBase,
        bottomCenter,
    };

    for (const QPoint& candidate : spawnCandidates) {
        if (!data.map->isInside(candidate))
            continue;

        Tile tile = data.map->tile(candidate);
        if (tile.type == TileType::Base)
            continue;

        if (tile.type != TileType::Empty)
            data.map->setTile(candidate, TileFactory::empty());

        data.playerSpawn = candidate;
        break;
    }

    return data;
}

LevelData LevelLoader::loadDefaultLevel(const GameRules& rules) const
{
    const QSize mapSize = rules.mapSize();
    const QPoint base = rules.baseCell();
    const qsizetype mapWidth = static_cast<qsizetype>(mapSize.width());
    const qsizetype mapHeight = static_cast<qsizetype>(mapSize.height());
    const qsizetype centerX = static_cast<qsizetype>(mapSize.width() / 2);
    const qsizetype lastY = mapHeight - 1;
    const qsizetype baseX = static_cast<qsizetype>(base.x());
    const qsizetype baseY = static_cast<qsizetype>(base.y());

    QStringList lines;
    lines.reserve(mapHeight);

    for (qsizetype y = 0; y < mapHeight; ++y) {
        QString row;
        row.reserve(mapWidth);
        for (qsizetype x = 0; x < mapWidth; ++x) {
            const QPoint cell(static_cast<int>(x), static_cast<int>(y));
            const bool border = (y == 0 || y == lastY || x == 0 || x == mapWidth - 1);
            const bool brickRow = (y % 4 == 2) && (x > 1) && (x < mapWidth - 2);
            const bool baseShield = (y >= baseY - 1 && y <= baseY) &&
                                    (x >= baseX - 1 && x <= baseX + 1);

            if (border) { row.append('S'); continue; }
            if (cell == base) { row.append('A'); continue; }
            if (x == baseX - 2 && y == baseY) { row.append('P'); continue; }
            if (y == 1 && (x == 1 || x == centerX || x == mapWidth - 2)) { row.append('E'); continue; }
            if (baseShield) { row.append('#'); continue; }
            if (brickRow) { row.append('#'); continue; }

            row.append('.');
        }
        lines.append(row);
    }

    return loadFromText(lines, rules);
}
