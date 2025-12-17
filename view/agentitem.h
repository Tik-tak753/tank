#ifndef AGENTITEM_H
#define AGENTITEM_H

#include <QGraphicsItem>

class Agent;

/*
 * Визуальное представление агента
 */
class AgentItem : public QGraphicsItem
{
public:
    explicit AgentItem(Agent* agent);

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    // синхронизация с моделью
    void syncFromAgent();

private:
    Agent* m_agent = nullptr;
    qreal m_size = 32.0;
};

#endif // AGENTITEM_H
