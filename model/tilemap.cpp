#include "tilemap.h"

TileMap::TileMap(int w, int h)
    : m_width(w), m_height(h), m_tiles(w * h, Empty)
{
    // рамка стен по краям
    for (int x = 0; x < w; ++x) {
        setWall({x, 0}, true);
        setWall({x, h - 1}, true);
    }
    for (int y = 0; y < h; ++y) {
        setWall({0, y}, true);
        setWall({w - 1, y}, true);
    }

    // тестовые внутренние стены
    for (int x = 4; x < 10; ++x)
        setWall({x, 6}, true);
}

int TileMap::index(int x, int y) const
{
    return y * m_width + x;
}

bool TileMap::isInside(QPoint p) const
{
    return p.x() >= 0 && p.y() >= 0 &&
           p.x() < m_width && p.y() < m_height;
}

TileMap::Tile TileMap::tile(QPoint p) const
{
    if (!isInside(p))
        return Wall;
    return m_tiles[index(p.x(), p.y())];
}

bool TileMap::isWalkable(QPoint p) const
{
    return tile(p) == Empty;
}

void TileMap::setWall(QPoint p, bool wall)
{
    if (!isInside(p))
        return;
    m_tiles[index(p.x(), p.y())] = wall ? Wall : Empty;
}
