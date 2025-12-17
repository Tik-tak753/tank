#include "world/Wall.h"

Wall::Wall(TileType type)
{
    m_tile = (type == TileType::Brick) ? TileFactory::brick() : TileFactory::steel();
}

bool Wall::isDestructible() const
{
    return m_tile.destructible;
}

Tile Wall::toTile() const
{
    return m_tile;
}
