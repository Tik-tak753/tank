#ifndef RENDERER_H
#define RENDERER_H

#include <QList>
#include <QPointF>

class QGraphicsScene;
class SpriteManager;
class Camera;
class Game;

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
    void drawMap(const Game& game);
    void drawTanks(const Game& game);
    void drawBullets(const Game& game);

    QGraphicsScene* m_scene = nullptr;
    SpriteManager* m_sprites = nullptr;
    Camera* m_camera = nullptr;
};

#endif // RENDERER_H
