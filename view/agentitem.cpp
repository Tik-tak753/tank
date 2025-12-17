#include "view/AgentItem.h"
#include "model/Agent.h"
#include "utils/Constants.h"

#include <QPainter>

AgentItem::AgentItem(Agent* agent)
    : m_agent(agent)
{
    setTransformOriginPoint(m_size / 2, m_size / 2);
}

QRectF AgentItem::boundingRect() const
{
    return QRectF(0, 0, m_size, m_size);
}

void AgentItem::paint(QPainter* painter,
                      const QStyleOptionGraphicsItem*,
                      QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setBrush(QColor(60, 140, 60));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boundingRect(), 6, 6);

    // ствол
    painter->setBrush(Qt::black);
    painter->drawRect(QRectF(m_size/2 - 2, -6, 4, 10));
}

void AgentItem::syncFromAgent()
{
    if (!m_agent)
        return;

    QPoint c = m_agent->cell();
    setPos(c.x() * m_size, c.y() * m_size);

    QPoint prev = m_agent->prevCell();
    QPoint dir = c - prev;

    if (dir == QPoint(1, 0))      setRotation(90);
    else if (dir == QPoint(-1,0)) setRotation(-90);
    else if (dir == QPoint(0, 1)) setRotation(180);
    else if (dir == QPoint(0,-1)) setRotation(0);
}
