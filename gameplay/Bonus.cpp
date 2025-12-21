#include "gameplay/Bonus.h"

#include <QPointF>

Bonus::Bonus(BonusType type, const QPoint& cell)
    : GameObject(QPointF(cell)),
      m_type(type)
{
}

void Bonus::collect()
{
    m_collected = true;
}

void Bonus::update(int deltaMs)
{
    GameObject::update(deltaMs);
}

StarBonus::StarBonus(const QPoint& cell)
    : Bonus(BonusType::Star, cell)
{
}
