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

    static constexpr int kMaxStars = 3;
    static constexpr int maxStars() { return kMaxStars; }

    int stars() const { return m_stars; }
    void addStar();
    bool canPierceSteel() const;

    void setInput(InputSystem* input);
    void setMap(const Map* map);

    void update() override;
    void updateWithDelta(int deltaMs) override;
    int bulletStepIntervalMs() const override;
    bool bulletCanPierceSteel() const override;
    bool receiveDamage(int dmg) override;

    void activateInvincibility(int durationMs);
    bool isInvincible() const { return m_invincibilityTimerMs > 0; }
    void tickBonusEffects(int deltaMs);

private:
    void applyUpgrades();
    int reloadTimeMs() const;
    void updateInvincibility(int deltaMs);

    InputSystem* m_input = nullptr;
    const Map* m_map = nullptr;
    bool m_sliding = false;
    int m_stars = 0;
    int m_invincibilityTimerMs = 0;
};

#endif // PLAYERTANK_H
