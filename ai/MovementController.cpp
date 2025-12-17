#include "ai/MovementController.h"

#include <QPoint>
#include <QRandomGenerator>

#include "gameplay/Direction.h"
#include "gameplay/EnemyTank.h"

void MovementController::tick(EnemyTank& tank)
{
    m_timerMs += 16;
    if (m_timerMs < m_intervalMs)
        return;

    m_timerMs = 0;
    const int dir = QRandomGenerator::global()->bounded(4);
    switch (dir) {
    case 0: tank.setDirection(Direction::Up); break;
    case 1: tank.setDirection(Direction::Down); break;
    case 2: tank.setDirection(Direction::Left); break;
    case 3: tank.setDirection(Direction::Right); break;
    }

    QPoint delta(0, 0);
    switch (tank.direction()) {
    case Direction::Up:    delta.setY(-1); break;
    case Direction::Down:  delta.setY(1);  break;
    case Direction::Left:  delta.setX(-1); break;
    case Direction::Right: delta.setX(1);  break;
    }
    tank.setCell(tank.cell() + delta);
}
