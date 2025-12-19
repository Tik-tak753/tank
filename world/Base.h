#ifndef BASE_H
#define BASE_H

#include <QPoint>

/*
 * Base (орел) — ціль, знищення якої завершує гру поразкою.
 */
class Base
{
public:
    explicit Base(const QPoint& cell);

    QPoint cell() const { return m_cell; }
    bool isDestroyed() const { return m_health <= 0; }
    int health() const { return m_health; }

    void takeDamage(int value = 1);

private:
    QPoint m_cell;
    int m_health = 2;
};

#endif // BASE_H
