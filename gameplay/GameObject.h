#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QPoint>
#include <QPointF>
#include <QRectF>

/*
 * GameObject — базовий клас для будь-якої сутності на карті.
 * Зберігає позицію у тайловій сітці та надає допоміжні методи
 * для майбутніх систем колізій та рендерингу.
 */
class GameObject
{
public:
    explicit GameObject(const QPointF& position = QPointF());
    virtual ~GameObject() = default;

    QPointF position() const { return m_position; }
    QPoint cell() const;

    void setPosition(const QPointF& pos);
    void setCell(const QPoint& cell);

    QPointF center() const;
    virtual QRectF boundingBox() const;

    virtual void update(int deltaMs);

protected:
    QPointF m_position;
};

#endif // GAMEOBJECT_H
