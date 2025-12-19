#ifndef RENDERER_H
#define RENDERER_H

#include <QHash>
#include <QList>
#include <QPointF>

class QGraphicsScene;
class QGraphicsItem;
class QGraphicsRectItem;
class SpriteManager;
class Camera;
class Game;
class Map;
class Tank;
class Bullet;

/*
 * Renderer відповідає за просту отрисовку кадру на QGraphicsScene.
 */
class Renderer
{
public:
    explicit Renderer(QGraphicsScene* scene);

    void setSpriteManager(SpriteManager* manager);
    void setCamera(Camera* camera);

    void renderFrame(const Game& game);

private:
    void initializeMap(const Game& game);
    void drawMap(const Game& game);
    void syncTanks(const Game& game);
    void syncBullets(const Game& game);
    void clearMapLayer();
    qreal tileSize() const;

    QGraphicsScene* m_scene = nullptr;
    SpriteManager* m_sprites = nullptr;
    Camera* m_camera = nullptr;

    const Map* m_cachedMap = nullptr;
    QList<QGraphicsItem*> m_mapItems;
    QHash<const Tank*, QGraphicsRectItem*> m_tankItems;
    QHash<const Bullet*, QGraphicsRectItem*> m_bulletItems;
};

#endif // RENDERER_H
