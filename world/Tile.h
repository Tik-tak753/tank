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

struct Tile
{
    TileType type = TileType::Empty;
    bool destructible = false;
    bool walkable = true;
};

// Утиліти створення стін для зручності
namespace TileFactory {
Tile brick();
Tile steel();
Tile empty();
Tile base();
}

#endif // TILE_H
