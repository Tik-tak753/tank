#include "world/LevelLoader.h"

#include <algorithm>

#include "world/Tile.h"
#include "world/Wall.h"

namespace {
QPoint defaultPlayerSpawn(const QSize& size)
{
    return QPoint(size.width() / 2, size.height() - 2);
}
}

LevelData LevelLoader::loadFromText(const QStringList& lines, const GameRules& rules) const
{
    LevelData data;
    const bool hasLines = !lines.isEmpty();
    const int height = hasLines ? lines.size() : rules.mapSize().height();
    int width = hasLines ? 0 : rules.mapSize().width();

    for (const QString& row : lines)
        width = std::max(width, row.size());

    const QSize mapSize(width, height);
    data.map = std::make_unique<Map>(mapSize);
    data.baseCell = rules.baseCell();
    data.playerSpawn = defaultPlayerSpawn(mapSize);

    // Легенда символів:
    // '.' або пробіл — порожня клітинка
    // '#' або 'B' — цегляна стіна
    // 'S' або 'X' — сталева стіна
    // 'A' або 'H' — база гравця
    // 'P' — позиція спавну гравця
    // 'E' — позиція спавну ворога
    for (int y = 0; y < lines.size(); ++y) {
        const QString& row = lines[y];
        for (int x = 0; x < row.size(); ++x) {
            const QChar symbol = row[x];
            const QPoint cell(x, y);

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

    return data;
}

LevelData LevelLoader::loadDefaultLevel(const GameRules& rules) const
{
    const QSize mapSize = rules.mapSize();
    const QPoint base = rules.baseCell();
    const int centerX = mapSize.width() / 2;
    const int lastY = mapSize.height() - 1;

    QStringList lines;
    lines.reserve(mapSize.height());

    for (int y = 0; y < mapSize.height(); ++y) {
        QString row;
        row.reserve(mapSize.width());
        for (int x = 0; x < mapSize.width(); ++x) {
            const bool border = (y == 0 || y == lastY || x == 0 || x == mapSize.width() - 1);
            const bool brickRow = (y % 4 == 2) && (x > 1) && (x < mapSize.width() - 2);
            const bool baseShield = (y >= base.y() - 1 && y <= base.y()) &&
                                    (x >= base.x() - 1 && x <= base.x() + 1);

            if (border) { row.append('S'); continue; }
            if (QPoint(x, y) == base) { row.append('A'); continue; }
            if (x == centerX && y == lastY - 1) { row.append('P'); continue; }
            if (y == 1 && (x == 1 || x == centerX || x == mapSize.width() - 2)) { row.append('E'); continue; }
            if (baseShield) { row.append('#'); continue; }
            if (brickRow) { row.append('#'); continue; }

            row.append('.');
        }
        lines.append(row);
    }

    return loadFromText(lines, rules);
}
