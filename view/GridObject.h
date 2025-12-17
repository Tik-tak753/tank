#ifndef GRIDOBJECT_H
#define GRIDOBJECT_H

#include <QGraphicsRectItem>
#include <QList>
#include <QPoint>
#include <QPointF>

class TileMap;

/*
 * Состояния объекта (FSM)
 */
enum class ObjectState {
    Idle,        // стоит
    Patrol,     // случайное движение
    FollowPath  // движение по пути (BFS)
};

class GridObject : public QGraphicsRectItem
{
public:
    GridObject(int gridX, int gridY, TileMap* map);

    // --- FSM ---
    void setState(ObjectState state);
    ObjectState state() const;

    // вызывается таймером
    void update();

    // --- BFS ---
    void setPath(const QList<QPoint>& path);

    // текущая клетка (для BFS старта)
    QPoint cell() const;

private:
    // --- шаги состояний ---
    void patrolStep();
    void followPathStep();

    // --- движение ---
    void tryMove(int newX, int newY);
    void updatePosition();

private:
    // целочисленные координаты клетки
    int m_gridX;
    int m_gridY;

    // плавная позиция (в пикселях)
    QPointF m_pos;

    // цель движения (центр следующей клетки)
    QPointF m_targetPos;

    // флаг движения
    bool m_moving = false;

    // скорость (пикселей за тик таймера)
    qreal m_speed = 4.0;

    TileMap* m_map = nullptr;

    // FSM
    ObjectState m_state = ObjectState::Idle;

    // путь для FollowPath
    QList<QPoint> m_path;
};

#endif // GRIDOBJECT_H
