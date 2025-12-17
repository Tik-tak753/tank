#ifndef AGENT_H
#define AGENT_H

#include <QPoint>
#include <QList>

class TileMap;

/*
 * Базовый агент (танк)
 * ЛОГИКА, БЕЗ ВИЗУАЛА
 */
class Agent
{
public:
    explicit Agent(int x, int y, TileMap* map);
    virtual ~Agent() = default;

    // позиция
    QPoint cell() const;
    QPoint prevCell() const;

    // обновление логики (FSM / движение)
    virtual void update();

    // путь (BFS)
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
