#ifndef TILE_H
#define TILE_H

#include <QPoint>

/*
 * Tile описує тип клітинки карти та базові властивості
 * прохідності/руйнування.
 */
enum class TileType {
    Empty,
    Brick,
    Steel,
    Water,
    Base
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
