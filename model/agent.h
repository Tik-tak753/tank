#ifndef AGENT_H
#define AGENT_H

#include <QPoint>
#include <QList>

class TileMap;

/*
 * Базовий агент (танк)
 * ЛОГІКА, БЕЗ ВІЗУАЛУ
 */
class Agent
{
public:
    explicit Agent(int x, int y, TileMap* map);
    virtual ~Agent() = default;

    // позиція
    QPoint cell() const;
    QPoint prevCell() const;

    // оновлення логіки (FSM / рух)
    virtual void update();

    // шлях (BFS)
    void setPath(const QList<QPoint>& path);

protected:
    void moveTo(int x, int y);

protected:
    TileMap* m_map = nullptr;

    QPoint m_cell;
    QPoint m_prevCell;

    QList<QPoint> m_path;
};

#endif // AGENT_H
