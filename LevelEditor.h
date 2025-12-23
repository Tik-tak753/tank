#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

#include <QColor>
#include <QRectF>
#include <QSize>
#include <QVector>
#include <QFont>
#include <optional>
#include <QString>

class Game;
class Map;
class QGraphicsView;
class QKeyEvent;
class QMouseEvent;
class QPoint;
class QPointF;
class QGraphicsRectItem;
class QGraphicsTextItem;

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
    void updateUi();

    bool handleKeyPress(QKeyEvent& event);
    bool handleMousePress(QMouseEvent& event);

private:
    bool isActive() const;
    Map* currentMap() const;
    void placeTile(const QPoint& cell, TileType type);
    bool isProtectedCell(const QPoint& cell) const;
    std::optional<QPoint> sceneToCell(const QPointF& scenePos) const;
    TileType selectedTileForKey(int key) const;
    TileType currentTileSelection() const;
    bool saveCurrentMap();
    bool loadMapFromFile();
    bool exportTiles(const QString& filePath) const;
    bool importTiles(const QString& filePath);
    void applyTileData(const QVector<QVector<int>>& rows, const QSize& declaredSize);
    void restoreProtectedCells(Map& map) const;
    QVector<QVector<int>> buildTileMatrix() const;
    int tileCode(TileType type) const;
    static std::optional<TileType> tileTypeFromCode(int code);
    void ensurePalette();
    void updatePaletteGeometry();
    void updatePaletteSelection();
    bool handlePaletteClick(const QPointF& scenePos);
    QColor tileColor(TileType type) const;
    QString tileLabel(TileType type) const;

    Game* m_game = nullptr;
    QGraphicsView* m_view = nullptr;
    TileType m_selectedType;
    QGraphicsRectItem* m_palettePanel = nullptr;
    QGraphicsRectItem* m_paletteSelection = nullptr;
    QVector<QGraphicsRectItem*> m_paletteSwatches;
    QVector<QGraphicsTextItem*> m_paletteLabels;
    QVector<TileType> m_paletteTypes;
    QRectF m_paletteBounds{0.0, 0.0, 0.0, 0.0};
    QFont m_paletteFont;
};

#endif // LEVEL_EDITOR_H
