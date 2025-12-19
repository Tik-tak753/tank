#include "systems/PhysicsSystem.h"

#include "gameplay/Bullet.h"
#include "world/Map.h"

void PhysicsSystem::update(QList<Bullet*>& bullets, const Map& map)
{
    for (Bullet* bullet : bullets)
        bullet->update(16, map);

    for (int i = bullets.size() - 1; i >= 0; --i) {
        if (!bullets.at(i)->isAlive()) {
            delete bullets.at(i);
            bullets.removeAt(i);
        }
    }
}
