#include "rendering/Renderer.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QPen>
#include <QSet>
#include <QtGlobal>

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

    initializeMap(game);
    syncTanks(game);
    syncBullets(game);
}

void Renderer::drawMap(const Game& game)
{
    const Map* map = game.map();
    if (!map)
        return;

    const qreal size = tileSize();
    const QSize mapSize = map->size();
    const qsizetype height = static_cast<qsizetype>(mapSize.height());
    const qsizetype width = static_cast<qsizetype>(mapSize.width());

    for (qsizetype y = 0; y < height; ++y) {
        for (qsizetype x = 0; x < width; ++x) {
            const QPoint cell(static_cast<int>(x), static_cast<int>(y));
            const Tile tile = map->tile(cell);
            if (tile.type == TileType::Empty)
                continue;

            QColor color = Qt::gray;
            if (tile.type == TileType::Brick)
                color = QColor(193, 68, 14);
            if (tile.type == TileType::Steel)
                color = QColor(160, 160, 160);
            if (tile.type == TileType::Base)
                color = QColor(230, 230, 0);

            const QPointF pos(static_cast<qreal>(x) * size, static_cast<qreal>(y) * size);
            QGraphicsRectItem* item = m_scene->addRect(QRectF(pos, QSizeF(size, size)), QPen(Qt::NoPen), QBrush(color));
            m_mapItems.append(item);
        }
    }
}

void Renderer::syncTanks(const Game& game)
{
    const qreal size = tileSize();
    QSet<const Tank*> seen;

    for (Tank* tank : game.tanks()) {
        if (!tank)
            continue;

        seen.insert(tank);
        QGraphicsRectItem* item = m_tankItems.value(tank, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(size, size)), QPen(Qt::black), QBrush(QColor(40, 160, 32)));
            m_tankItems.insert(tank, item);
        }

        const QPointF pos = QPointF(tank->cell()) * size;
        item->setPos(pos);
    }

    auto it = m_tankItems.begin();
    while (it != m_tankItems.end()) {
        if (!seen.contains(it.key())) {
            delete it.value();
            it = m_tankItems.erase(it);
        } else {
            ++it;
        }
    }
}

void Renderer::syncBullets(const Game& game)
{
    const qreal size = tileSize();
    const qreal bulletSize = size / 2.0;
    QSet<const Bullet*> seen;

    for (Bullet* bullet : game.bullets()) {
        if (!bullet)
            continue;

        seen.insert(bullet);
        QGraphicsRectItem* item = m_bulletItems.value(bullet, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(bulletSize, bulletSize)), QPen(Qt::NoPen), QBrush(Qt::yellow));
            m_bulletItems.insert(bullet, item);
        }

        const QPointF pos = QPointF(bullet->cell()) * size;
        item->setPos(pos);
    }

    auto it = m_bulletItems.begin();
    while (it != m_bulletItems.end()) {
        if (!seen.contains(it.key())) {
            delete it.value();
            it = m_bulletItems.erase(it);
        } else {
            ++it;
        }
    }
}

void Renderer::initializeMap(const Game& game)
{
    const Map* map = game.map();
    if (m_cachedMap != map) {
        clearMapLayer();
        m_cachedMap = map;
        drawMap(game);
    }
}

void Renderer::clearMapLayer()
{
    for (QGraphicsItem* item : m_mapItems)
        delete item;
    m_mapItems.clear();
}

qreal Renderer::tileSize() const
{
    return static_cast<qreal>(m_camera ? m_camera->tileSize() : TILE_SIZE);
}
