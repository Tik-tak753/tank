#include "model/BotAgent.h"
#include "model/TileMap.h"

#include <QRandomGenerator>

/* =====================
 * Constructor
 * ===================== */

BotAgent::BotAgent(int x, int y, TileMap* map)
    : Agent(x, y, map)
{
}

/* =====================
 * Update
 * ===================== */

void BotAgent::update()
{
    // если есть путь — идём по нему
    if (!m_path.isEmpty()) {
        Agent::update();
        return;
    }

    // иначе — патруль
    patrol();
}

/* =====================
 * Patrol behaviour
 * ===================== */

void BotAgent::patrol()
{
    if (!m_map)
        return;

    int dir = QRandomGenerator::global()->bounded(4);

    int nx = m_cell.x();
    int ny = m_cell.y();

    if (dir == 0) nx--;
    if (dir == 1) nx++;
    if (dir == 2) ny--;
    if (dir == 3) ny++;

    QPoint next(nx, ny);
    if (!m_map->isWalkable(next))
        return;

    m_prevCell = m_cell;
    m_cell = next;
}
