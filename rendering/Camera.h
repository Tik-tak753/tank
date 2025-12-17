#ifndef CAMERA_H
#define CAMERA_H

#include <QPointF>

/*
 * Camera конвертує координати світу у координати сцени.
 */
class Camera
{
public:
    void setTileSize(int size) { m_tileSize = size; }
    int tileSize() const { return m_tileSize; }

    QPointF toScene(const QPointF& worldPos) const;

private:
    int m_tileSize = 32;
};

#endif // CAMERA_H
