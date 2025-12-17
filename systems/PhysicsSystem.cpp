#include "systems/PhysicsSystem.h"

#include <algorithm>

#include "gameplay/Bullet.h"
#include "world/Map.h"

void PhysicsSystem::update(QList<Bullet*>& bullets, const Map& map)
{
    Q_UNUSED(map);

    for (Bullet* bullet : bullets)
        bullet->update(16);

    for (int i = bullets.size() - 1; i >= 0; --i) {
        if (bullets.at(i)->isExpired()) {
            delete bullets.at(i);
            bullets.removeAt(i);
        }
    }
}
