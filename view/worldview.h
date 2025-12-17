#ifndef WORLDVIEW_H
#define WORLDVIEW_H

#include <QGraphicsView>
#include <QList>
#include <QPoint>
#include <Qt>

class TileMap;
class QGraphicsRectItem;
class QGraphicsItem;

class WorldView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit WorldView(QGraphicsScene* scene, QWidget* parent = nullptr);

    // отрисовка карты (без удаления акторов)
    void drawMap(const TileMap& map, int cellSize);

    // подсветка BFS-пути
    void showPath(const QList<QPoint>& path);
    void clearPath();

signals:
    // единый сигнал мыши
    void cellClicked(const QPoint& cell, Qt::MouseButton button);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    int m_cellSize = 32;

    // элементы карты (сетка + стены)
    QList<QGraphicsItem*> m_mapItems;

    // элементы подсветки пути
    QList<QGraphicsRectItem*> m_pathItems;
};

#endif // WORLDVIEW_H

