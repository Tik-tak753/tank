#ifndef GRIDOBJECT_H
#define GRIDOBJECT_H

#include <QGraphicsRectItem>
#include <QList>
#include <QPoint>
#include <QPointF>

class TileMap;

/*
 * Стани об'єкта (FSM)
 */
enum class ObjectState {
    Idle,        // стоїть
    Patrol,      // випадковий рух
    FollowPath   // рух за шляхом (BFS)
};

class GridObject : public QGraphicsRectItem
{
public:
    GridObject(int gridX, int gridY, TileMap* map);

    // --- FSM ---
    void setState(ObjectState state);
    ObjectState state() const;

    // викликається таймером
    void update();

    // --- BFS ---
    void setPath(const QList<QPoint>& path);

    // поточна клітинка (для BFS старту)
    QPoint cell() const;

private:
    // --- кроки станів ---
    void patrolStep();
    void followPathStep();

    // --- рух ---
    void tryMove(int newX, int newY);
    void updatePosition();

private:
    // цілочисельні координати клітинки
    int m_gridX;
    int m_gridY;

    // плавна позиція (у пікселях)
    QPointF m_pos;

    // ціль руху (центр наступної клітинки)
    QPointF m_targetPos;

    // прапорець руху
    bool m_moving = false;

    // швидкість (пікселів за тик таймера)
    qreal m_speed = 4.0;

    TileMap* m_map = nullptr;

    // FSM
    ObjectState m_state = ObjectState::Idle;

    // шлях для FollowPath
    QList<QPoint> m_path;
};

#endif // GRIDOBJECT_H
