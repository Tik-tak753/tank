#include "view/WorldView.h"
#include "model/TileMap.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>

/* =========================
 * Constructor
 * ========================= */

WorldView::WorldView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    setRenderHint(QPainter::Antialiasing, false);
}

/* =========================
 * Отрисовка карты
 * (ТОЛЬКО карта, без акторов)
 * ========================= */

void WorldView::drawMap(const TileMap& map, int cellSize)
{
    m_cellSize = cellSize;

    // удалить ТОЛЬКО старые элементы карты
    for (QGraphicsItem* item : m_mapItems) {
        scene()->removeItem(item);
        delete item;
    }
    m_mapItems.clear();

    // и подсветку пути
    clearPath();

    QPen gridPen(Qt::lightGray);
    QBrush wallBrush(Qt::darkGray);

    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            QRectF r(x * cellSize, y * cellSize, cellSize, cellSize);

            // сетка
            QGraphicsRectItem* grid =
                scene()->addRect(r, gridPen);
            grid->setZValue(-10);
            m_mapItems.append(grid);

            // стена
            if (!map.isWalkable({x, y})) {
                QGraphicsRectItem* wall =
                    scene()->addRect(r, Qt::NoPen, wallBrush);
                wall->setZValue(-9);
                m_mapItems.append(wall);
            }
        }
    }
}

/* =========================
 * Подсветка BFS-пути
 * ========================= */

void WorldView::clearPath()
{
    for (QGraphicsRectItem* item : m_pathItems) {
        scene()->removeItem(item);
        delete item;
    }
    m_pathItems.clear();
}

void WorldView::showPath(const QList<QPoint>& path)
{
    clearPath();

    QBrush pathBrush(QColor(0, 200, 255, 120));
    QPen noPen(Qt::NoPen);

    for (const QPoint& cell : path) {
        QRectF r(
            cell.x() * m_cellSize,
            cell.y() * m_cellSize,
            m_cellSize,
            m_cellSize
            );

        QGraphicsRectItem* rect =
            scene()->addRect(r, noPen, pathBrush);
        rect->setZValue(-5); // над картой, под объектами
        m_pathItems.append(rect);
    }
}

/* =========================
 * Mouse input
 * ========================= */

void WorldView::mousePressEvent(QMouseEvent* event)
{
    QPointF scenePos = mapToScene(event->pos());

    int x = int(scenePos.x()) / m_cellSize;
    int y = int(scenePos.y()) / m_cellSize;

    emit cellClicked(QPoint(x, y), event->button());
}
