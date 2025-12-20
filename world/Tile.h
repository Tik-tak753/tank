#ifndef TILE_H
#define TILE_H

#include <QPoint>

/*
 * Tile описує тип клітинки карти та базові властивості
 * прохідності/руйнування.
 */
enum class TileType {
    Empty,
    Brick,      // цегляна стіна — руйнується
    Steel,      // сталева стіна — не руйнується
    Base        // база гравця
};

class Tile
{
public:
    TileType type = TileType::Empty;
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
}

#endif // TILE_H
