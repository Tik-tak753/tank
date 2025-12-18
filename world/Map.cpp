#include "world/Map.h"

Map::Map(int width, int height)
    : m_size(width, height),
      m_tiles(height, QVector<Tile>(width, TileFactory::empty()))
{
}

bool Map::isInside(const QPoint& cell) const
{
    return cell.x() >= 0 && cell.y() >= 0 &&
           cell.x() < m_size.width() && cell.y() < m_size.height();
}

Tile Map::tile(const QPoint& cell) const
{
    if (!isInside(cell))
        return TileFactory::steel();
    return m_tiles[cell.y()][cell.x()];
}

void Map::setTile(const QPoint& cell, const Tile& tile)
{
    if (!isInside(cell))
        return;
    m_tiles[cell.y()][cell.x()] = tile;
}

bool Map::isWalkable(const QPoint& cell) const
{
    return tile(cell).walkable;
}
