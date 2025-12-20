#include "gameplay/GameObject.h"

#include <QSizeF>
#include <QtMath>

GameObject::GameObject(const QPointF& position)
    : m_position(position)
{
}

QPoint GameObject::cell() const
{
    return QPoint(qFloor(m_position.x()), qFloor(m_position.y()));
}

void GameObject::setPosition(const QPointF& pos)
{
    m_position = pos;
}

void GameObject::setCell(const QPoint& cell)
{
    m_position = QPointF(cell);
}

QPointF GameObject::center() const
{
    return m_position + QPointF(0.5, 0.5);
}

QRectF GameObject::boundingBox() const
{
    return QRectF(m_position, QSizeF(1.0, 1.0));
}

void GameObject::update(int deltaMs)
{
    Q_UNUSED(deltaMs);
}
