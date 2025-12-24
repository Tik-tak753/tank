#ifndef BONUS_H
#define BONUS_H

#include "gameplay/GameObject.h"

class Game;
class PlayerTank;

enum class BonusType {
    Star,
    Helmet,
    Clock,
    Grenade
};

/*
 * Bonus — базовий клас для підбираних об'єктів на карті.
 * Він не блокує рух та дозволяє описувати бонуси даними,
 * щоб розширювати систему новими типами без зміни інших підсистем.
 */
class Bonus : public GameObject
{
public:
    Bonus(const QPoint& cell, BonusType type);
    ~Bonus() override = default;

    BonusType type() const { return m_type; }
    bool isCollected() const { return m_collected; }
    void collect();
    // Застосовує ефект бонусу до гри й гравця
    virtual void apply(Game& game, PlayerTank& player) = 0;

    // Мерехтіння/таймер життя бонусу
    void update(int deltaMs) override;

private:
    BonusType m_type;
    bool m_collected = false;
};

class StarBonus : public Bonus
{
public:
    explicit StarBonus(const QPoint& cell);

    void apply(Game& game, PlayerTank& player) override;
};

class HelmetBonus : public Bonus
{
public:
    explicit HelmetBonus(const QPoint& cell);

    void apply(Game& game, PlayerTank& player) override;
};

class ClockBonus : public Bonus
{
public:
    explicit ClockBonus(const QPoint& cell);

    void apply(Game& game, PlayerTank& player) override;
};

class GrenadeBonus : public Bonus
{
public:
    explicit GrenadeBonus(const QPoint& cell);

    void apply(Game& game, PlayerTank& player) override;
};

#endif // BONUS_H
