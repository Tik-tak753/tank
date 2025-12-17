#ifndef PLAYERTANK_H
#define PLAYERTANK_H

#include "gameplay/Tank.h"

class InputSystem;

/*
 * PlayerTank — обгортка над Tank з інтеграцією клавіатурного вводу.
 */
class PlayerTank : public Tank
{
public:
    explicit PlayerTank(const QPoint& cell);

    void setInput(InputSystem* input);
    void update() override;

private:
    InputSystem* m_input = nullptr;
};

#endif // PLAYERTANK_H
