#ifndef HUDITEM_H
#define HUDITEM_H

#include <QFont>
#include <QGraphicsItem>
#include <QPointF>
#include <QRectF>
#include <QString>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

// HudItem малює панель життів, зірок, ворогів і очок поверх сцени
class HudItem : public QGraphicsItem
{
public:
    HudItem();

    void setMetrics(int lives, int stars, int maxStars, int enemies, int score, const QString& status, qreal tileSize);

    QRectF boundingRect() const override { return m_bounds; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    void updateFont(qreal tileSize);
    void updateBounds();
    void drawTankIcon(QPainter* painter, const QPointF& topLeft, qreal size) const;
    void drawStarIcon(QPainter* painter, const QPointF& center, qreal outerRadius, bool filled) const;
    void drawEnemyIcon(QPainter* painter, const QPointF& topLeft, qreal size) const;

    int m_lives = 0;
    int m_stars = 0;
    int m_maxStars = 0;
    int m_enemies = 0;
    int m_score = 0;
    QString m_statusText;

    qreal m_tileSize = 0.0;
    qreal m_labelWidth = 0.0;
    qreal m_lineHeight = 0.0;
    qreal m_iconSize = 0.0;
    QRectF m_bounds{0.0, 0.0, 1.0, 1.0};
    QFont m_font;
};

#endif // HUDITEM_H
