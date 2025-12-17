#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QList>
#include <QPoint>

class TileMap;

class PathFinder
{
public:
    static QList<QPoint> findPath(
        const TileMap& map,
        QPoint start,
        QPoint goal
        );
};

#endif // PATHFINDER_H
