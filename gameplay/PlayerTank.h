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

    // Оновлення стану зчитує ввід і застосовує бонуси
    void update() override;
    // Оновлює рух із часовою дельтою
    void updateWithDelta(int deltaMs) override;
    // Перерахунок швидкості кулі залежно від зірок
    int bulletStepIntervalMs() const override;
    // Дозвіл пробивати сталь після апгрейдів
    bool bulletCanPierceSteel() const override;
    // Враховує невразливість від бонусів
    bool receiveDamage(int dmg) override;

    void activateInvincibility(int durationMs);
    bool isInvincible() const { return m_invincibilityTimerMs > 0; }
    void tickBonusEffects(int deltaMs);

private:
    float movementSpeedTilesPerSecond() const;
    int bulletStepIntervalForStars() const;
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
