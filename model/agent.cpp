#include "model/Agent.h"
#include "model/TileMap.h"

Agent::Agent(int x, int y, TileMap* map)
    : m_map(map),
    m_cell(x, y),
    m_prevCell(x, y)
{
}

QPoint Agent::cell() const
{
    return m_cell;
}

QPoint Agent::prevCell() const
{
    return m_prevCell;
}

void Agent::setPath(const QList<QPoint>& path)
{
    m_path = path;
}

void Agent::update()
{
    // минимально: следование пути
    if (m_path.isEmpty())
        return;

    QPoint next = m_path.takeFirst();
    moveTo(next.x(), next.y());
}

void Agent::moveTo(int x, int y)
{
    if (!m_map)
        return;

    QPoint next(x, y);
    if (!m_map->isWalkable(next))
        return;

    m_prevCell = m_cell;
    m_cell = next;
}
