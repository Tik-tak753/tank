#ifndef TILEMAP_H
#define TILEMAP_H

#include <QVector>
#include <QPoint>

class TileMap
{
public:
    enum Tile {
        Empty = 0,
        Wall  = 1
    };

    TileMap(int w, int h);

    int width()  const { return m_width; }
    int height() const { return m_height; }

    bool isInside(QPoint p) const;
    bool isWalkable(QPoint p) const;

    void setWall(QPoint p, bool wall);
    Tile tile(QPoint p) const;

private:
    int m_width;
    int m_height;
    QVector<Tile> m_tiles;

    int index(int x, int y) const;
};

#endif
