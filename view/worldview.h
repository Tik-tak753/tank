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

    // малювання карти (без видалення акторів)
    void drawMap(const TileMap& map, int cellSize);

    // підсвітка BFS-шляху
    void showPath(const QList<QPoint>& path);
    void clearPath();

signals:
    // єдиний сигнал кліка миші
    void cellClicked(const QPoint& cell, Qt::MouseButton button);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    int m_cellSize = 32;

    // елементи карти (сітка + стіни)
    QList<QGraphicsItem*> m_mapItems;

    // елементи підсвітки шляху
    QList<QGraphicsRectItem*> m_pathItems;
};

#endif // WORLDVIEW_H

