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
    explicit Map(const QSize& size) : Map(size.width(), size.height()) {}

    QSize size() const { return m_size; }
    bool isInside(const QPoint& cell) const;
    Tile tile(const QPoint& cell) const;
    void setTile(const QPoint& cell, const Tile& tile);
    bool isWalkable(const QPoint& cell) const;

private:
    QSize m_size;
    QVector<QVector<Tile>> m_tiles; // m_tiles[y][x]
};

#endif // MAP_H
