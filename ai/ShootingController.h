#ifndef SHOOTINGCONTROLLER_H
#define SHOOTINGCONTROLLER_H

class EnemyTank;

/*
 * ShootingController контролює частоту пострілів ворога.
 */
class ShootingController
{
public:
    void tick(EnemyTank& tank);

private:
    int m_cooldownMs = 0;
    int m_reloadMs = 900;
};

#endif // SHOOTINGCONTROLLER_H
