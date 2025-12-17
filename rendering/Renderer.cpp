#include "rendering/Renderer.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QPen>

#include "core/Game.h"
#include "gameplay/Bullet.h"
#include "gameplay/Tank.h"
#include "rendering/Camera.h"
#include "rendering/SpriteManager.h"
#include "utils/Constants.h"
#include "world/Base.h"
#include "world/Map.h"
#include "world/Tile.h"

Renderer::Renderer(QGraphicsScene* scene)
    : m_scene(scene)
{
}

void Renderer::setSpriteManager(SpriteManager* manager)
{
    m_sprites = manager;
}

void Renderer::setCamera(Camera* camera)
{
    m_camera = camera;
}

void Renderer::renderFrame(const Game& game)
{
    if (!m_scene)
        return;

    m_scene->clear();
    drawMap(game);
    drawTanks(game);
    drawBullets(game);
}

void Renderer::drawMap(const Game& game)
{
    const Map* map = game.map();
    if (!map)
        return;

    const int tileSize = m_camera ? m_camera->tileSize() : TILE_SIZE;
    for (int y = 0; y < map->size().height(); ++y) {
        for (int x = 0; x < map->size().width(); ++x) {
            const Tile tile = map->tile(QPoint(x, y));
            if (tile.type == TileType::Empty)
                continue;

            QColor color = Qt::gray;
            if (tile.type == TileType::Brick)
                color = QColor(193, 68, 14);
            if (tile.type == TileType::Steel)
                color = QColor(160, 160, 160);
            if (tile.type == TileType::Base)
                color = QColor(230, 230, 0);

            QPointF pos(x * tileSize, y * tileSize);
            m_scene->addRect(QRectF(pos, QSizeF(tileSize, tileSize)), QPen(Qt::NoPen), QBrush(color));
        }
    }
}

void Renderer::drawTanks(const Game& game)
{
    const int tileSize = m_camera ? m_camera->tileSize() : TILE_SIZE;
    for (Tank* tank : game.tanks()) {
        QPointF pos = QPointF(tank->cell()) * tileSize;
        QColor color = QColor(40, 160, 32);
        m_scene->addRect(QRectF(pos, QSizeF(tileSize, tileSize)), QPen(Qt::black), QBrush(color));
    }
}

void Renderer::drawBullets(const Game& game)
{
    const int tileSize = m_camera ? m_camera->tileSize() : TILE_SIZE;
    for (Bullet* bullet : game.bullets()) {
        QPointF pos = bullet->position();
        if (m_camera)
            pos = m_camera->toScene(pos);
        else
            pos *= tileSize;
        m_scene->addRect(QRectF(pos, QSizeF(tileSize / 4, tileSize / 4)), QPen(Qt::NoPen), QBrush(Qt::yellow));
    }
}
