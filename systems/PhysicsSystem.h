#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include <QList>

class Bullet;
class Map;

/*
 * PhysicsSystem відповідає за оновлення руху снарядів та час їх життя.
 */
class PhysicsSystem
{
public:
    void update(QList<Bullet*>& bullets, const Map& map);
};

#endif // PHYSICSSYSTEM_H
