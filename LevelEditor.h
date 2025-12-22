#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

#include <optional>

class Game;
class Map;
class QGraphicsView;
class QKeyEvent;
class QMouseEvent;
class QPoint;
class QPointF;

enum class TileType;

/*
 * LevelEditor — мінімальний модуль для редагування тайлів карти.
 * Відповідає лише за перетворення вводу користувача у виклики Map::setTile.
 */
class LevelEditor
{
public:
    LevelEditor();

    void setGame(Game* game);
    void setView(QGraphicsView* view);

    bool handleKeyPress(QKeyEvent& event);
    bool handleMousePress(QMouseEvent& event);

private:
    bool isActive() const;
    Map* currentMap() const;
    void placeTile(const QPoint& cell, TileType type);
    std::optional<QPoint> sceneToCell(const QPointF& scenePos) const;
    TileType selectedTileForKey(int key) const;
    TileType currentTileSelection() const;

    Game* m_game = nullptr;
    QGraphicsView* m_view = nullptr;
    TileType m_selectedType;
};

#endif // LEVEL_EDITOR_H
