#ifndef BONUS_H
#define BONUS_H

#include "gameplay/GameObject.h"

enum class BonusType {
    Star
};

/*
 * Bonus — базовий клас для підбираних об'єктів на карті.
 * Він не блокує рух та дозволяє описувати бонуси даними,
 * щоб розширювати систему новими типами без зміни інших підсистем.
 */
class Bonus : public GameObject
{
public:
    Bonus(BonusType type, const QPoint& cell);
    ~Bonus() override = default;

    BonusType type() const { return m_type; }
    bool isCollected() const { return m_collected; }
    void collect();

    void update(int deltaMs) override;

private:
    BonusType m_type;
    bool m_collected = false;
};

class StarBonus : public Bonus
{
public:
    explicit StarBonus(const QPoint& cell);
};

#endif // BONUS_H
