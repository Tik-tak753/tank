#ifndef WORLD_H
#define WORLD_H

#include <QList>

class Agent;
class TileMap;

/*
 * World — модель мира
 * Хранит карту и список агентов
 */
class World
{
public:
    explicit World(TileMap* map);
    ~World();

    // агенты
    void addAgent(Agent* agent);
    const QList<Agent*>& agents() const;

    // шаг симуляции
    void update();

    // доступ к карте
    TileMap* map() const;

private:
    TileMap* m_map = nullptr;
    QList<Agent*> m_agents;
};

#endif // WORLD_H
