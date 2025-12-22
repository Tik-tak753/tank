#include "LevelEditor.h"

#include <QGraphicsView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <QSize>
#include <QtGlobal>
#include <Qt>
#include <algorithm>

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

bool LevelEditor::handleKeyPress(QKeyEvent& event)
{
    if (!isActive())
        return false;

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

    const QPoint viewportPos = m_view->viewport()->mapFromGlobal(event.globalPos());
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

    const Tile existingTile = map->tile(cell);
    if (existingTile.type == TileType::Base || type == TileType::Base)
        return;

    map->setTile(cell, tileForType(type));
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
