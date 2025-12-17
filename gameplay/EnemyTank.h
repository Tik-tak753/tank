#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include <memory>

#include "gameplay/Tank.h"

class EnemyAI;

/*
 * EnemyTank — представлення ворога, який керується AI.
 */
class EnemyTank : public Tank
{
public:
    explicit EnemyTank(const QPoint& cell);
    ~EnemyTank();

    void update() override;

private:
    std::unique_ptr<EnemyAI> m_ai;
};

#endif // ENEMYTANK_H
