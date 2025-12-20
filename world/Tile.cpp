#include "world/Tile.h"

#include <QtGlobal>
#include <limits>

int Tile::maxDamage() const
{
    switch (type) {
    case TileType::Brick:
        return 4;
    case TileType::Steel:
        return std::numeric_limits<int>::max();
    case TileType::Empty:
    case TileType::Base:
        return 0;
    }

    Q_UNREACHABLE();
}

void Tile::takeDamage(int amount)
{
    if (amount <= 0)
        return;

    const int limit = maxDamage();
    if (limit <= 0)
        return;

    m_damage = qBound(0, m_damage + amount, limit);
}

bool Tile::isDestroyed() const
{
    const int limit = maxDamage();
    if (limit <= 0)
        return false;

    return m_damage >= limit;
}

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
