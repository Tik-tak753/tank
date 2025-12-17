#include "view/GridObject.h"
#include "model/TileMap.h"
#include "utils/Constants.h"

#include <QBrush>
#include <QRandomGenerator>
#include <QtMath>

/* =======================
 * Constructor
 * ======================= */

GridObject::GridObject(int gridX, int gridY, TileMap* map)
    : m_gridX(gridX)
    , m_gridY(gridY)
    , m_map(map)
{
    setBrush(QBrush(Qt::blue));

    m_pos = QPointF(
        m_gridX * TILE_SIZE,
        m_gridY * TILE_SIZE
        );

    setRect(0, 0, TILE_SIZE, TILE_SIZE);
    setPos(m_pos);
}

/* =======================
 * FSM interface
 * ======================= */

void GridObject::setState(ObjectState state)
{
    m_state = state;
}

ObjectState GridObject::state() const
{
    return m_state;
}

/* =======================
 * Cell access
 * ======================= */

QPoint GridObject::cell() const
{
    return QPoint(m_gridX, m_gridY);
}

/* =======================
 * Update (FSM core)
 * ======================= */

void GridObject::update()
{
    switch (m_state) {
    case ObjectState::Idle:
        break;

    case ObjectState::Patrol:
        patrolStep();
        break;

    case ObjectState::FollowPath:
        followPathStep();
        break;
    }
}

/* =======================
 * Patrol behaviour
 * ======================= */

void GridObject::patrolStep()
{
    if (m_moving)
        return;

    int dir = QRandomGenerator::global()->bounded(4);

    if (dir == 0) tryMove(m_gridX - 1, m_gridY);
    if (dir == 1) tryMove(m_gridX + 1, m_gridY);
    if (dir == 2) tryMove(m_gridX, m_gridY - 1);
    if (dir == 3) tryMove(m_gridX, m_gridY + 1);
}

/* =======================
 * FollowPath behaviour
 * ======================= */

void GridObject::setPath(const QList<QPoint>& path)
{
    m_path = path;
    m_moving = false;

    if (!m_path.isEmpty())
        setState(ObjectState::FollowPath);
}

void GridObject::followPathStep()
{
    // если сейчас едем — продолжаем интерполяцию
    if (m_moving) {
        QPointF delta = m_targetPos - m_pos;
        qreal dist = QLineF(QPointF(0,0), delta).length();

        if (dist <= m_speed) {
            // доехали до цели
            m_pos = m_targetPos;
            setPos(m_pos);
            m_moving = false;
        } else {
            QPointF step = (delta / dist) * m_speed;
            m_pos += step;
            setPos(m_pos);
        }
        return;
    }

    // если путь закончился
    if (m_path.isEmpty()) {
        setState(ObjectState::Idle);
        return;
    }

    // берём следующую клетку
    QPoint next = m_path.takeFirst();

    if (!m_map->isWalkable(next)) {
        setState(ObjectState::Idle);
        return;
    }

    m_gridX = next.x();
    m_gridY = next.y();

    m_targetPos = QPointF(
        m_gridX * TILE_SIZE,
        m_gridY * TILE_SIZE
        );

    m_moving = true;
}

/* =======================
 * Movement helpers
 * ======================= */

void GridObject::tryMove(int newX, int newY)
{
    if (!m_map)
        return;

    QPoint next(newX, newY);
    if (!m_map->isWalkable(next))
        return;

    m_gridX = newX;
    m_gridY = newY;

    m_targetPos = QPointF(
        m_gridX * TILE_SIZE,
        m_gridY * TILE_SIZE
        );

    m_moving = true;
}
