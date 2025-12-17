#ifndef MAP_H
#define MAP_H

#include <QPoint>
#include <QSize>
#include <QVector>

#include "world/Tile.h"

/*
 * Map зберігає сітку Tile та допоміжні методи
 * для колізій і модифікації клітинок.
 */
class Map
{
public:
    Map(int width, int height);

    QSize size() const { return m_size; }
    bool isInside(const QPoint& cell) const;
    Tile tile(const QPoint& cell) const;
    void setTile(const QPoint& cell, const Tile& tile);
    bool isWalkable(const QPoint& cell) const;

private:
    int index(const QPoint& cell) const;

    QSize m_size;
    QVector<Tile> m_tiles;
};

#endif // MAP_H
