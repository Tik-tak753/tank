#ifndef ENUMS_H
#define ENUMS_H

enum TankType
{
    Player = 0,
    Enemy
};

struct TileCollisionResult
{
    bool destroyBullet = false;
    bool destroyTile = false;
    bool damageBase = false;
};

#endif // ENUMS_H
