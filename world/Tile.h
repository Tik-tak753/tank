#ifndef TILE_H
#define TILE_H

#include <QPoint>
#include <cstdint>

/*
 * Tile описує тип клітинки карти та базові властивості
 * прохідності/руйнування.
 */
enum class TileType {
    Empty,
    Brick,      // цегляна стіна — руйнується
    Steel,      // сталева стіна — не руйнується
    Base,       // база гравця
    Forest,
    Water,
    Ice
};

enum CollisionMask : std::uint8_t {
    BlockNone   = 0,
    BlockTank   = 1 << 0,
    BlockBullet = 1 << 1
};

class Tile
{
public:
    TileType type = TileType::Empty;
    CollisionMask blockMask = BlockNone;
    bool destructible = false;
    bool walkable = true;

    int damage() const { return m_damage; }
    int maxDamage() const;
    void takeDamage(int amount);
    bool isDestroyed() const;

    bool isSteel() const { return type == TileType::Steel; }

private:
    int m_damage = 0;        // використовується для Brick / Steel
};

// Утиліти створення стін для зручності
namespace TileFactory {
Tile brick();
Tile steel();
Tile empty();
Tile base();
Tile forest();
Tile water();
Tile ice();
}

#endif // TILE_H
