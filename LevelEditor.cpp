#include "LevelEditor.h"

#include <QFile>
#include <QFileDialog>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QObject>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <QSize>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>
#include <QtGlobal>
#include <Qt>
#include <algorithm>
#include <array>

#include "core/Game.h"
#include "core/GameState.h"
#include "world/Map.h"
#include "world/Tile.h"

namespace {
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
} // namespace

LevelEditor::LevelEditor()
    : m_selectedType(TileType::Brick)
{
}

void LevelEditor::setGame(Game* game)
{
    m_game = game;
}

void LevelEditor::setView(QGraphicsView* view)
{
    m_view = view;
}

TileType LevelEditor::selectedTile() const
{
    return m_selectedType;
}

void LevelEditor::setSelectedTile(TileType type)
{
    if (type == TileType::Base)
        return;

    m_selectedType = type;
}

bool LevelEditor::handleKeyPress(QKeyEvent& event)
{
    if (!isActive())
        return false;

    const bool ctrlHeld = event.modifiers() & Qt::ControlModifier;
    if (ctrlHeld && event.key() == Qt::Key_S) {
        saveCurrentMap();
        event.accept();
        return true;
    }

    if (ctrlHeld && (event.key() == Qt::Key_O || event.key() == Qt::Key_L)) {
        loadMapFromFile();
        event.accept();
        return true;
    }

    const TileType nextType = selectedTileForKey(event.key());
    if (nextType == m_selectedType)
        return false;

    if (nextType != m_selectedType) {
        m_selectedType = nextType;
        event.accept();
        return true;
    }

    return false;
}

bool LevelEditor::handleMousePress(QMouseEvent& event)
{
    if (!isActive())
        return false;

    const Qt::MouseButton button = event.button();
    if (button != Qt::LeftButton && button != Qt::RightButton)
        return false;

    if (!m_view || !m_view->viewport())
        return false;

    const QPoint viewportPos =
        m_view->viewport()->mapFromGlobal(event.globalPosition().toPoint());
    if (!m_view->viewport()->rect().contains(viewportPos))
        return false;

    const QPointF scenePos = m_view->mapToScene(viewportPos);
    const std::optional<QPoint> cell = sceneToCell(scenePos);
    if (!cell.has_value())
        return false;

    const TileType type = (button == Qt::RightButton) ? TileType::Empty : currentTileSelection();
    placeTile(*cell, type);
    event.accept();
    return true;
}

bool LevelEditor::isActive() const
{
    return m_game
        && m_view
        && m_game->state().gameMode() == GameMode::Editing
        && currentMap();
}

Map* LevelEditor::currentMap() const
{
    return m_game ? m_game->map() : nullptr;
}

void LevelEditor::placeTile(const QPoint& cell, TileType type)
{
    Map* map = currentMap();
    if (!map)
        return;

    if (!map->isInside(cell))
        return;

    if (isProtectedCell(cell))
        return;

    const Tile existingTile = map->tile(cell);
    if (existingTile.type == TileType::Base || type == TileType::Base)
        return;

    map->setTile(cell, tileForType(type));
}

bool LevelEditor::isProtectedCell(const QPoint& cell) const
{
    Map* map = currentMap();
    if (!map)
        return false;

    if (!map->isInside(cell))
        return false;

    const QSize mapSize = map->size();
    const QPoint baseCell = m_game ? m_game->rules().baseCell() : QPoint();
    const QPoint playerSpawn = defaultPlayerSpawn(mapSize, baseCell);
    const QPoint leftOfBase(baseCell.x() - 1, baseCell.y());
    const QPoint bottomCenter(std::clamp(mapSize.width() / 2, 0, mapSize.width() - 1),
                              std::clamp(mapSize.height() - 2, 0, mapSize.height() - 1));
    const std::array<QPoint, 3> playerCandidates = {playerSpawn, leftOfBase, bottomCenter};

    for (const QPoint& candidate : playerCandidates) {
        if (!map->isInside(candidate))
            continue;

        const Tile tile = map->tile(candidate);
        if (tile.type == TileType::Base)
            continue;

        if (cell == candidate)
            return true;

        break;
    }

    const std::array<QPoint, 3> enemySpawns = defaultEnemySpawns(mapSize);
    for (const QPoint& spawn : enemySpawns) {
        if (!map->isInside(spawn))
            continue;

        if (cell == spawn)
            return true;
    }

    return false;
}

std::optional<QPoint> LevelEditor::sceneToCell(const QPointF& scenePos) const
{
    Map* map = currentMap();
    if (!map || !m_view || !m_view->viewport())
        return std::nullopt;

    const QSize viewportSize = m_view->viewport()->size();
    const QSize mapSize = map->size();
    if (viewportSize.isEmpty() || mapSize.isEmpty())
        return std::nullopt;

    const qreal viewportWidth = static_cast<qreal>(viewportSize.width());
    const qreal viewportHeight = static_cast<qreal>(viewportSize.height());
    const qreal mapWidthTiles = static_cast<qreal>(mapSize.width());
    const qreal mapHeightTiles = static_cast<qreal>(mapSize.height());
    const qreal scale = std::min(viewportWidth / mapWidthTiles, viewportHeight / mapHeightTiles);
    if (scale <= 0.0)
        return std::nullopt;

    const QPointF offset((viewportWidth - mapWidthTiles * scale) / 2.0,
                         (viewportHeight - mapHeightTiles * scale) / 2.0);
    const QPointF localPos = scenePos - offset;
    const int cellX = static_cast<int>(localPos.x() / scale);
    const int cellY = static_cast<int>(localPos.y() / scale);
    const QPoint cell(cellX, cellY);

    if (!map->isInside(cell))
        return std::nullopt;

    return cell;
}

TileType LevelEditor::selectedTileForKey(int key) const
{
    switch (key) {
    case Qt::Key_1: return TileType::Empty;
    case Qt::Key_2: return TileType::Brick;
    case Qt::Key_3: return TileType::Steel;
    case Qt::Key_4: return TileType::Forest;
    case Qt::Key_5: return TileType::Water;
    case Qt::Key_6: return TileType::Ice;
    case Qt::Key_7: return m_selectedType;
    default:
        break;
    }

    return m_selectedType;
}

TileType LevelEditor::currentTileSelection() const
{
    return m_selectedType;
}

bool LevelEditor::saveCurrentMap()
{
    if (!isActive())
        return false;

    const QString filePath = QFileDialog::getSaveFileName(
        m_view, QObject::tr("Save Map"), QString(), QObject::tr("Text Files (*.txt);;All Files (*)"));
    if (filePath.isEmpty())
        return false;

    return exportTiles(filePath);
}

bool LevelEditor::loadMapFromFile()
{
    if (!isActive())
        return false;

    const QString filePath = QFileDialog::getOpenFileName(
        m_view, QObject::tr("Load Map"), QString(), QObject::tr("Text Files (*.txt);;All Files (*)"));
    if (filePath.isEmpty())
        return false;

    return importTiles(filePath);
}

bool LevelEditor::exportTiles(const QString& filePath) const
{
    Map* map = currentMap();
    if (!map)
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    const QSize size = map->size();
    stream << size.width() << " " << size.height() << "\n";

    const QVector<QVector<int>> matrix = buildTileMatrix();
    for (const QVector<int>& row : matrix) {
        for (qsizetype i = 0; i < row.size(); ++i) {
            if (i > 0)
                stream << " ";
            stream << row.at(i);
        }
        stream << "\n";
    }

    return true;
}

bool LevelEditor::importTiles(const QString& filePath)
{
    Map* map = currentMap();
    if (!map)
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    QVector<QVector<int>> rows;
    QSize declaredSize = map->size();
    bool sizeParsed = false;
    int lineIndex = 0;

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            ++lineIndex;
            continue;
        }

        const QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (!sizeParsed && parts.size() >= 2) {
            bool okWidth = false;
            bool okHeight = false;
            const int width = parts.at(0).toInt(&okWidth);
            const int height = parts.at(1).toInt(&okHeight);
            if (okWidth && okHeight && width > 0 && height > 0) {
                declaredSize = QSize(width, height);
                sizeParsed = true;
                ++lineIndex;
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

        ++lineIndex;
    }

    applyTileData(rows, declaredSize);
    return true;
}

void LevelEditor::applyTileData(const QVector<QVector<int>>& rows, const QSize& declaredSize)
{
    Map* map = currentMap();
    if (!map)
        return;

    Q_UNUSED(declaredSize);

    const QSize mapSize = map->size();
    const int width = mapSize.width();
    const int height = mapSize.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const QPoint cell(x, y);
            if (isProtectedCell(cell))
                continue;
            map->setTile(cell, TileFactory::empty());
        }
    }

    const int appliedHeight = std::min(height, static_cast<int>(rows.size()));
    for (int y = 0; y < appliedHeight; ++y) {
        const QVector<int>& row = rows.at(y);
        const int appliedWidth = std::min(width, static_cast<int>(row.size()));
        for (int x = 0; x < appliedWidth; ++x) {
            const QPoint cell(x, y);
            if (isProtectedCell(cell))
                continue;

            const int code = row.at(x);
            const std::optional<TileType> type = tileTypeFromCode(code);
            if (!type.has_value() || *type == TileType::Base)
                continue;

            map->setTile(cell, tileForType(*type));
        }
    }

    restoreProtectedCells(*map);
}

void LevelEditor::restoreProtectedCells(Map& map) const
{
    const QSize mapSize = map.size();
    const QPoint baseCell = m_game ? m_game->rules().baseCell() : QPoint();
    if (map.isInside(baseCell))
        map.setTile(baseCell, TileFactory::base());

    const QPoint playerSpawn = defaultPlayerSpawn(mapSize, baseCell);
    const QPoint leftOfBase(baseCell.x() - 1, baseCell.y());
    const QPoint bottomCenter(std::clamp(mapSize.width() / 2, 0, mapSize.width() - 1),
                              std::clamp(mapSize.height() - 2, 0, mapSize.height() - 1));
    const std::array<QPoint, 3> playerCandidates = {playerSpawn, leftOfBase, bottomCenter};
    for (const QPoint& candidate : playerCandidates) {
        if (!map.isInside(candidate))
            continue;
        if (map.tile(candidate).type == TileType::Base)
            continue;
        map.setTile(candidate, TileFactory::empty());
        break;
    }

    const std::array<QPoint, 3> enemySpawns = defaultEnemySpawns(mapSize);
    for (const QPoint& spawn : enemySpawns) {
        if (!map.isInside(spawn))
            continue;
        if (map.tile(spawn).type == TileType::Base)
            continue;
        map.setTile(spawn, TileFactory::empty());
    }
}

QVector<QVector<int>> LevelEditor::buildTileMatrix() const
{
    QVector<QVector<int>> rows;
    Map* map = currentMap();
    if (!map)
        return rows;

    const QSize mapSize = map->size();
    rows.resize(mapSize.height());

    for (int y = 0; y < mapSize.height(); ++y) {
        QVector<int>& row = rows[y];
        row.resize(mapSize.width());
        for (int x = 0; x < mapSize.width(); ++x) {
            const QPoint cell(x, y);
            const Tile tile = map->tile(cell);
            if (tile.type == TileType::Base || isProtectedCell(cell)) {
                row[x] = tileCode(TileType::Empty);
                continue;
            }
            row[x] = tileCode(tile.type);
        }
    }

    return rows;
}

int LevelEditor::tileCode(TileType type) const
{
    switch (type) {
    case TileType::Empty: return 0;
    case TileType::Brick: return 1;
    case TileType::Steel: return 2;
    case TileType::Forest: return 3;
    case TileType::Water: return 4;
    case TileType::Ice: return 5;
    case TileType::Base: return -1;
    }

    return 0;
}

std::optional<TileType> LevelEditor::tileTypeFromCode(int code)
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
