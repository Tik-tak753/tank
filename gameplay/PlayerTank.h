#ifndef PLAYERTANK_H
#define PLAYERTANK_H

#include "gameplay/Tank.h"

class InputSystem;
class Map;

/*
 * PlayerTank — обгортка над Tank з інтеграцією клавіатурного вводу.
 */
class PlayerTank : public Tank
{
public:
    explicit PlayerTank(const QPoint& cell);

    void setInput(InputSystem* input);
    void setMap(const Map* map);

    void update() override;
    void updateWithDelta(int deltaMs) override;

private:
    InputSystem* m_input = nullptr;
    const Map* m_map = nullptr;
    float m_moveAccumulator = 0.0f;
    bool m_sliding = false;
}; 

#endif // PLAYERTANK_H
