#include "rendering/Camera.h"

QPointF Camera::toScene(const QPointF& worldPos) const
{
    return QPointF(worldPos.x() * m_tileSize, worldPos.y() * m_tileSize);
}
