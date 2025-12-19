#include "rendering/Renderer.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QFont>
#include <QPen>
#include <QSet>
#include <QtGlobal>

#include "core/Game.h"
#include "gameplay/Bullet.h"
#include "gameplay/Tank.h"
#include "gameplay/EnemyTank.h"
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

    clearMapLayer();     // DEBUG ONLY
    drawMap(game);       // reflect runtime tile changes

    syncTanks(game);
    syncBullets(game);
    updateHud(game);
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
            item->setZValue(0);
            m_mapItems.append(item);
        }
    }
}

void Renderer::syncTanks(const Game& game)
{
    const qreal size = tileSize();
    const qreal barrelLength = size * 0.6;
    const qreal barrelThickness = size * 0.2;
    QSet<const Tank*> seen;

    auto barrelRectForDirection = [&](Direction dir) {
        switch (dir) {
        case Direction::Up:
            return QRectF((size - barrelThickness) / 2.0, 0, barrelThickness, barrelLength);
        case Direction::Down:
            return QRectF((size - barrelThickness) / 2.0, size - barrelLength, barrelThickness, barrelLength);
        case Direction::Left:
            return QRectF(0, (size - barrelThickness) / 2.0, barrelLength, barrelThickness);
        case Direction::Right:
            return QRectF(size - barrelLength, (size - barrelThickness) / 2.0, barrelLength, barrelThickness);
        }

        return QRectF();
    };

    for (Tank* tank : game.tanks()) {
        if (!tank)
            continue;

        seen.insert(tank);
        QGraphicsRectItem* item = m_tankItems.value(tank, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(size, size)), QPen(Qt::black), QBrush(QColor(40, 160, 32)));
            item->setZValue(10);
            m_tankItems.insert(tank, item);
        }

        QGraphicsRectItem* directionItem = m_tankDirectionItems.value(tank, nullptr);
        if (!directionItem) {
            directionItem = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(barrelThickness, barrelLength)), QPen(Qt::NoPen), QBrush(Qt::black));
            directionItem->setZValue(11);
            m_tankDirectionItems.insert(tank, directionItem);
        }

        const QPointF pos = QPointF(tank->cell()) * size;
        item->setPos(pos);
        directionItem->setRect(barrelRectForDirection(tank->direction()));
        directionItem->setPos(pos);
    }

    auto it = m_tankItems.begin();
    while (it != m_tankItems.end()) {
        if (!seen.contains(it.key())) {
            QGraphicsItem* item = it.value();
            m_scene->removeItem(item);
            delete item;
            it = m_tankItems.erase(it);
        } else {
            ++it;
        }
    }

    auto dirIt = m_tankDirectionItems.begin();
    while (dirIt != m_tankDirectionItems.end()) {
        if (!seen.contains(dirIt.key())) {
            QGraphicsItem* item = dirIt.value();
            m_scene->removeItem(item);
            delete item;
            dirIt = m_tankDirectionItems.erase(dirIt);
        } else {
            ++dirIt;
        }
    }
}

void Renderer::syncBullets(const Game& game)
{
    const qreal size = tileSize();
    const qreal bulletSize = size / 2.0;
    QSet<const Bullet*> seen;

    const QPointF bulletOffset((size - bulletSize) / 2.0, (size - bulletSize) / 2.0);

    for (Bullet* bullet : game.bullets()) {
        if (!bullet || !bullet->isAlive())
            continue;

        seen.insert(bullet);
        QGraphicsRectItem* item = m_bulletItems.value(bullet, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(bulletSize, bulletSize)), QPen(Qt::NoPen), QBrush(Qt::yellow));
            item->setZValue(20);
            m_bulletItems.insert(bullet, item);
        }

        const QPointF pos = QPointF(bullet->cell()) * size + bulletOffset;
        item->setPos(pos);
    }

    auto it = m_bulletItems.begin();
    while (it != m_bulletItems.end()) {
        if (!seen.contains(it.key())) {
            QGraphicsItem* item = it.value();
            m_scene->removeItem(item);
            delete item;
            it = m_bulletItems.erase(it);
        } else {
            ++it;
        }
    }
}

void Renderer::updateHud(const Game& game)
{
    if (!m_scene)
        return;

    if (!m_hudItem) {
        m_hudItem = m_scene->addText(QString());
        m_hudItem->setDefaultTextColor(Qt::white);
        QFont font = m_hudItem->font();
        font.setPointSize(16);
        m_hudItem->setFont(font);
        m_hudItem->setZValue(100);
        m_hudItem->setPos(10, 10);
    }

    const int lives = game.state().remainingLives();

    int enemyCount = 0;
    for (Tank* tank : game.tanks()) {
        if (dynamic_cast<EnemyTank*>(tank))
            ++enemyCount;
    }

    const QString text = QStringLiteral("LIVES: %1\nENEMIES: %2")
                             .arg(lives)
                             .arg(enemyCount);

    if (m_hudItem->toPlainText() != text)
        m_hudItem->setPlainText(text);
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
    for (QGraphicsItem* item : m_mapItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_mapItems.clear();
}

qreal Renderer::tileSize() const
{
    return static_cast<qreal>(m_camera ? m_camera->tileSize() : TILE_SIZE);
}
