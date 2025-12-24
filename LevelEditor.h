#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

#include <QSize>
#include <QVector>
#include <optional>
#include <QString>

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

    TileType selectedTile() const;
    void setSelectedTile(TileType type);

    // Обробляє гарячі клавіші вибору тайлу та збереження/завантаження
    bool handleKeyPress(QKeyEvent& event);
    // Конвертує клік миші у координати клітинки та ставить тайл
    bool handleMousePress(QMouseEvent& event);

private:
    // Редактор активний лише коли гра у режимі Editing та є карта
    bool isActive() const;
    // Поточна карта, з якою працює редактор
    Map* currentMap() const;
    // Записує тайл у карту з перевірками захищених клітинок
    void placeTile(const QPoint& cell, TileType type);
    // Визначає, чи клітинку не можна змінювати (база/спавни)
    bool isProtectedCell(const QPoint& cell) const;
    // Перетворює координати сцени на індекс клітинки
    std::optional<QPoint> sceneToCell(const QPointF& scenePos) const;
    // Призначає вибір тайлу за натисненою цифрою
    TileType selectedTileForKey(int key) const;
    TileType currentTileSelection() const;
    // Зберігає карту у файл через діалог
    bool saveCurrentMap();
    // Завантажує карту з текстового файлу
    bool loadMapFromFile();
    // Експортує матрицю тайлів у текст
    bool exportTiles(const QString& filePath) const;
    // Імпортує матрицю тайлів з тексту
    bool importTiles(const QString& filePath);
    // Застосовує розібрані коди тайлів до поточної карти
    void applyTileData(const QVector<QVector<int>>& rows, const QSize& declaredSize);
    // Повертає захищені клітинки до валідного стану
    void restoreProtectedCells(Map& map) const;
    // Будує матрицю чисел для поточної карти
    QVector<QVector<int>> buildTileMatrix() const;
    // Відображення типу тайлу у числовий код файлу
    int tileCode(TileType type) const;
    static std::optional<TileType> tileTypeFromCode(int code);

    Game* m_game = nullptr;
    QGraphicsView* m_view = nullptr;
    TileType m_selectedType;
};

#endif // LEVEL_EDITOR_H
