#ifndef CAMERA_H
#define CAMERA_H

#include <QPointF>

/*
 * Camera конвертує координати світу у координати сцени.
 */
class Camera
{
public:
    void setTileSize(qreal size) { m_tileSize = size; }
    qreal tileSize() const { return m_tileSize; }

    QPointF toScene(const QPointF& worldPos) const;

private:
    qreal m_tileSize = 32.0;
};

#endif // CAMERA_H
