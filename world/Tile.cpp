#include "world/Tile.h"

Tile TileFactory::brick()
{
    Tile t;
    t.type = TileType::Brick;
    t.destructible = true;
    t.walkable = false;
    return t;
}

Tile TileFactory::steel()
{
    Tile t;
    t.type = TileType::Steel;
    t.destructible = false;
    t.walkable = false;
    return t;
}

Tile TileFactory::empty()
{
    Tile t;
    t.type = TileType::Empty;
    t.destructible = false;
    t.walkable = true;
    return t;
}

Tile TileFactory::base()
{
    Tile t;
    t.type = TileType::Base;
    t.destructible = true;
    t.walkable = false;
    return t;
}
