#include "systems/PhysicsSystem.h"

#include "gameplay/Bullet.h"
#include "world/Map.h"

void PhysicsSystem::update(QList<Bullet*>& bullets, const Map& map, int deltaMs)
{
    for (Bullet* bullet : bullets) {
        if (!bullet)
            continue;
        bullet->update(deltaMs, map);
    }
}
