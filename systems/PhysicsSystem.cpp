#include "systems/PhysicsSystem.h"

#include "gameplay/Bullet.h"

void PhysicsSystem::update(QList<Bullet*>& bullets, int deltaMs)
{
    for (Bullet* bullet : bullets) {
        if (!bullet)
            continue;
        bullet->update(deltaMs);
    }
}
