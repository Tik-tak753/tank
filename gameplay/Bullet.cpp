#include "gameplay/Bullet.h"

Bullet::Bullet(const QPointF& pos, Direction dir, float speed)
    : m_position(pos),
      m_direction(dir),
      m_speed(speed)
{
}

void Bullet::update(int deltaMs)
{
    m_elapsedMs += deltaMs;
    QPointF delta(0, 0);
    const float deltaSeconds = deltaMs / 1000.0f;
    switch (m_direction) {
    case Direction::Up:    delta.setY(-m_speed * deltaSeconds); break;
    case Direction::Down:  delta.setY(m_speed * deltaSeconds);  break;
    case Direction::Left:  delta.setX(-m_speed * deltaSeconds); break;
    case Direction::Right: delta.setX(m_speed * deltaSeconds);  break;
    }
    m_position += delta;
}

bool Bullet::isExpired() const
{
    return m_elapsedMs >= m_lifetimeMs;
}
