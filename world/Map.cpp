#include "world/Map.h"

Map::Map(int width, int height)
    : m_size(width, height),
      m_width(static_cast<qsizetype>(width)),
      m_height(static_cast<qsizetype>(height)),
      m_tiles(m_height, QVector<Tile>(m_width, TileFactory::empty()))
{
}

bool Map::isInside(const QPoint& cell) const
{
    const int xInt = cell.x();
    const int yInt = cell.y();
    if (xInt < 0 || yInt < 0)
        return false;

    const qsizetype x = static_cast<qsizetype>(xInt);
    const qsizetype y = static_cast<qsizetype>(yInt);
    return x < m_width && y < m_height;
}

Tile Map::tile(const QPoint& cell) const
{
    if (!isInside(cell))
        return TileFactory::steel();

    const qsizetype x = static_cast<qsizetype>(cell.x());
    const qsizetype y = static_cast<qsizetype>(cell.y());
    return m_tiles.at(y).at(x);
}

void Map::setTile(const QPoint& cell, const Tile& tile)
{
    if (!isInside(cell))
        return;

    const qsizetype x = static_cast<qsizetype>(cell.x());
    const qsizetype y = static_cast<qsizetype>(cell.y());
    m_tiles[y][x] = tile;
}

bool Map::isWalkable(const QPoint& cell) const
{
    // Рух танків обмежується лише маскою BlockTank.
    // Таким чином дані карти вирішують, які клітинки є стінами, без гілок у коді руху.
    const Tile target = tile(cell);
    return !(target.blockMask & BlockTank);
}
