#ifndef RENDERER_H
#define RENDERER_H

#include <QHash>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QSet>

class QGraphicsScene;
class QGraphicsItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class SpriteManager;
class Camera;
class Game;
class Map;
class Tank;
class Bullet;

struct Explosion
{
    QPoint cell;
    int ttlFrames;
};

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
    void updateExplosions();
    void updateHud(const Game& game);
    void updateBaseBlinking(const Game& game);
    void clearMapLayer();
    qreal tileSize() const;

    QGraphicsScene* m_scene = nullptr;
    SpriteManager* m_sprites = nullptr;
    Camera* m_camera = nullptr;

    const Map* m_cachedMap = nullptr;
    QList<QGraphicsItem*> m_mapItems;
    QHash<const Tank*, QGraphicsRectItem*> m_tankItems;
    QHash<const Tank*, QGraphicsRectItem*> m_tankDirectionItems;
    QHash<const Bullet*, QGraphicsRectItem*> m_bulletItems;
    QList<QGraphicsRectItem*> m_explosionItems;
    QGraphicsTextItem* m_hudLivesItem = nullptr;
    QGraphicsTextItem* m_hudEnemiesItem = nullptr;

    bool m_baseBlinking = false;
    int m_baseBlinkCounter = 0;
    int m_lastBaseHealth = -1;

    QList<Explosion> m_explosions;
    QSet<const Bullet*> m_previousBullets;
    QHash<const Bullet*, QPoint> m_lastBulletCells;
    QSet<const Tank*> m_destroyedTanks;
};

#endif // RENDERER_H
