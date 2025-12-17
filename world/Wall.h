#ifndef WALL_H
#define WALL_H

#include "world/Tile.h"

/*
 * Wall інкапсулює параметри цегляних/сталевих перешкод.
 */
class Wall
{
public:
    explicit Wall(TileType type);

    bool isDestructible() const;
    Tile toTile() const;

private:
    Tile m_tile;
};

#endif // WALL_H
