#ifndef BOTAGENT_H
#define BOTAGENT_H

#include "model/Agent.h"

class TileMap;

/*
 * BotAgent — агент с простым поведением
 */
class BotAgent : public Agent
{
public:
    BotAgent(int x, int y, TileMap* map);

    void update() override;

private:
    void patrol();
};

#endif // BOTAGENT_H
