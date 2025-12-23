#include "world/LevelLoader.h"

#include <algorithm>
#include <array>
#include <optional>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

#include "world/Tile.h"
#include "world/Wall.h"

namespace {
QPoint defaultPlayerSpawn(const QSize& size, const QPoint& baseCell)
{
    const QPoint leftOfBase(baseCell.x() - 2, baseCell.y());
    if (leftOfBase.x() >= 0 && leftOfBase.x() < size.width() &&
        leftOfBase.y() >= 0 && leftOfBase.y() < size.height()) {
        return leftOfBase;
    }

    return QPoint(std::clamp(size.width() / 2, 0, size.width() - 1),
                  std::clamp(size.height() - 2, 0, size.height() - 1));
}

std::array<QPoint, 3> defaultEnemySpawns(const QSize& size)
{
    const int maxX = std::max(0, size.width() - 1);
    const int maxY = std::max(0, size.height() - 1);
    const int y = std::clamp(1, 0, maxY);

    const int left = std::clamp(1, 0, maxX);
    const int center = std::clamp(size.width() / 2, 0, maxX);
    const int right = std::clamp(size.width() - 2, 0, maxX);

    return {QPoint(left, y), QPoint(center, y), QPoint(right, y)};
}

Tile tileForType(TileType type)
{
    switch (type) {
    case TileType::Empty: return TileFactory::empty();
    case TileType::Brick: return TileFactory::brick();
    case TileType::Steel: return TileFactory::steel();
    case TileType::Forest: return TileFactory::forest();
    case TileType::Water: return TileFactory::water();
    case TileType::Ice: return TileFactory::ice();
    case TileType::Base: return TileFactory::base();
    }

    Q_UNREACHABLE();
}

std::optional<TileType> tileTypeFromCode(int code)
{
    switch (code) {
    case 0: return TileType::Empty;
    case 1: return TileType::Brick;
    case 2: return TileType::Steel;
    case 3: return TileType::Forest;
    case 4: return TileType::Water;
    case 5: return TileType::Ice;
    default:
        break;
    }

    return std::nullopt;
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

LevelData LevelLoader::loadSavedLevel(const GameRules& rules) const
{
    LevelData fallback = loadDefaultLevel(rules);

    const QString relativePath = QStringLiteral("assets/maps/Demo.txt");
    qInfo() << "LevelLoader: current working directory:" << QDir::currentPath();
    const QString applicationDir = QCoreApplication::applicationDirPath();
    qInfo() << "LevelLoader: applicationDirPath:" << applicationDir;

    auto logCandidate = [](const QString& label, const QString& path) {
        const QFileInfo info(path);
        qInfo() << "LevelLoader:" << label << ":" << path
                << "exists:" << info.exists() << "readable:" << info.isReadable();
    };

    const QString initialPath = QFileInfo(relativePath).absoluteFilePath();
    logCandidate(QStringLiteral("initial candidate (working directory)"), initialPath);

    const QList<QString> candidatePaths = {
        QDir(applicationDir).absoluteFilePath(relativePath),
        QDir(applicationDir).absoluteFilePath(QStringLiteral("../%1").arg(relativePath)),
        QDir(applicationDir).absoluteFilePath(QStringLiteral("../../%1").arg(relativePath)),
        QDir(applicationDir).absoluteFilePath(QStringLiteral("../../../%1").arg(relativePath)),
        initialPath,
    };

    QString resolvedPath;
    for (const QString& candidate : candidatePaths) {
        logCandidate(QStringLiteral("candidate path"), candidate);
        const QFileInfo info(candidate);
        if (info.exists() && info.isReadable()) {
            resolvedPath = info.absoluteFilePath();
            break;
        }
    }

    if (resolvedPath.isEmpty()) {
        resolvedPath = initialPath;
        qWarning() << "LevelLoader: no readable saved level found; using initial path"
                   << resolvedPath;
    }

    qInfo() << "LevelLoader: resolved path to saved level:" << resolvedPath;

    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "LevelLoader: failed to open saved level:" << file.errorString();
        qWarning() << "LevelLoader: falling back to default level";
        return fallback;
    }

    QTextStream stream(&file);
    QVector<QVector<int>> rows;
    QSize declaredSize = rules.mapSize();
    bool sizeParsed = false;

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;

        const QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (!sizeParsed && parts.size() >= 2) {
            bool okWidth = false;
            bool okHeight = false;
            const int width = parts.at(0).toInt(&okWidth);
            const int height = parts.at(1).toInt(&okHeight);
            if (okWidth && okHeight && width > 0 && height > 0) {
                declaredSize = QSize(width, height);
                sizeParsed = true;
                continue;
            }
        }

        QVector<int> row;
        row.reserve(parts.size());
        for (const QString& part : parts) {
            bool ok = false;
            const int value = part.toInt(&ok);
            if (!ok)
                continue;
            row.append(value);
        }

        if (!row.isEmpty())
            rows.append(std::move(row));
    }

    if (!sizeParsed && rows.isEmpty())
        return fallback;

    const QSize mapSize = rules.mapSize();
    LevelData data;
    data.map = std::make_unique<Map>(mapSize);
    data.baseCell = rules.baseCell();
    data.playerSpawn = defaultPlayerSpawn(mapSize, data.baseCell);
    const std::array<QPoint, 3> spawnDefaults = defaultEnemySpawns(mapSize);
    for (const QPoint& spawn : spawnDefaults)
        data.enemySpawns.append(spawn);
    data.loadedFromFile = true;

    const int width = mapSize.width();
    const int height = mapSize.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const QPoint cell(x, y);
            data.map->setTile(cell, TileFactory::empty());
        }
    }

    const int appliedHeight = std::min({height, declaredSize.height(), static_cast<int>(rows.size())});
    for (int y = 0; y < appliedHeight; ++y) {
        const QVector<int>& row = rows.at(y);
        const int appliedWidth = std::min({width, declaredSize.width(), static_cast<int>(row.size())});
        for (int x = 0; x < appliedWidth; ++x) {
            const int code = row.at(x);
            const std::optional<TileType> type = tileTypeFromCode(code);
            if (!type.has_value() || *type == TileType::Base)
                continue;

            const QPoint cell(x, y);
            data.map->setTile(cell, tileForType(*type));
        }
    }

    if (data.map->isInside(data.baseCell))
        data.map->setTile(data.baseCell, TileFactory::base());

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

        data.map->setTile(candidate, TileFactory::empty());
        data.playerSpawn = candidate;
        break;
    }

    const std::array<QPoint, 3> enemySpawns = defaultEnemySpawns(mapSize);
    data.enemySpawns.clear();
    for (const QPoint& spawn : enemySpawns) {
        if (!data.map->isInside(spawn))
            continue;
        if (data.map->tile(spawn).type == TileType::Base)
            continue;
        data.map->setTile(spawn, TileFactory::empty());
        data.enemySpawns.append(spawn);
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

    LevelData data = loadFromText(lines, rules);

    const QPoint steelCell(5, 5);
    if (data.map->isInside(steelCell))
        data.map->setTile(steelCell, TileFactory::steel());

    const QPoint forestCell(6, 5);
    if (data.map->isInside(forestCell))
        data.map->setTile(forestCell, TileFactory::forest());

    return data;
}
