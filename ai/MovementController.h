#ifndef MOVEMENTCONTROLLER_H
#define MOVEMENTCONTROLLER_H

class EnemyTank;

/*
 * MovementController відповідає за просту зміну напрямку ворога.
 */
class MovementController
{
public:
    void tick(EnemyTank& tank);

private:
    int m_timerMs = 0;
    int m_intervalMs = 700;
};

#endif // MOVEMENTCONTROLLER_H
