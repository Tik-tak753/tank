#include "PathFinder.h"
#include "model/TileMap.h"

#include <QQueue>
#include <QHash>
#include <QSet>

QList<QPoint> PathFinder::findPath(
    const TileMap& map,
    QPoint start,
    QPoint goal)
{
    QQueue<QPoint> frontier;
    QSet<QPoint> visited;
    QHash<QPoint, QPoint> cameFrom;

    frontier.enqueue(start);
    visited.insert(start);

    const QPoint dirs[4] = {
        { 1, 0 },
        {-1, 0 },
        { 0, 1 },
        { 0,-1 }
    };

    while (!frontier.isEmpty()) {
        QPoint current = frontier.dequeue();

        if (current == goal)
            break;

        for (const QPoint& d : dirs) {
            QPoint next = current + d;

            if (!map.isWalkable(next))
                continue;

            if (visited.contains(next))
                continue;

            visited.insert(next);
            cameFrom[next] = current;
            frontier.enqueue(next);
        }
    }

    QList<QPoint> path;
    if (!cameFrom.contains(goal))
        return path;

    QPoint cur = goal;
    while (cur != start) {
        path.prepend(cur);
        cur = cameFrom[cur];
    }
    path.prepend(start);

    return path;
}
