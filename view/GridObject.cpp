#include "view/GridObject.h"
#include "model/TileMap.h"
#include "utils/Constants.h"

#include <QBrush>
#include <QRandomGenerator>
#include <QtMath>

/* =======================
 * Конструктор
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
 * Інтерфейс FSM
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
 * Доступ до клітинки
 * ======================= */

QPoint GridObject::cell() const
{
    return QPoint(m_gridX, m_gridY);
}

/* =======================
 * Оновлення (ядро FSM)
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
 * Патрулювання
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
 * Поведінка руху за шляхом
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
    // якщо вже рухаємося — продовжуємо інтерполяцію
    if (m_moving) {
        QPointF delta = m_targetPos - m_pos;
        qreal dist = QLineF(QPointF(0,0), delta).length();

        if (dist <= m_speed) {
            // доїхали до цілі
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

    // якщо шлях закінчився
    if (m_path.isEmpty()) {
        setState(ObjectState::Idle);
        return;
    }

    // беремо наступну клітинку
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
 * Допоміжні функції руху
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
