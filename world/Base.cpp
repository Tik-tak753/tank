#include "world/Base.h"

Base::Base(const QPoint& cell)
    : m_cell(cell)
{
}

void Base::takeDamage(int value)
{
    m_health -= value;
}
