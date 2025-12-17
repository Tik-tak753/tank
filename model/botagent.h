#ifndef BOTAGENT_H
#define BOTAGENT_H

#include "model/Agent.h"

class TileMap;

/*
 * BotAgent — агент із простою поведінкою
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
