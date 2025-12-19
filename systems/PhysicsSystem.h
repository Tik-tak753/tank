#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include <QList>

class Bullet;
/*
 * PhysicsSystem відповідає за оновлення руху снарядів та час їх життя.
 */
class PhysicsSystem
{
public:
    void update(QList<Bullet*>& bullets, int deltaMs);
};

#endif // PHYSICSSYSTEM_H
