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
#include <QDebug>
#include <algorithm>
#include <utility>

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

    updateBaseBlinking(game);
    initializeMap(game);
    syncMapTiles(game);

    syncTanks(game);
    syncBullets(game);
    updateExplosions();
    updateHud(game);
}

void Renderer::drawMap(const Game& game)
{
    const Map* map = game.map();
    if (!map)
        return;
    const Base* base = game.base();
    const QPoint baseCell = base ? base->cell() : QPoint(-1, -1);
    const bool baseDestroyed = base && base->isDestroyed();
    const bool blinkPhase = m_baseBlinking && ((m_baseBlinkCounter / 8) % 2 == 0);

    const qreal size = tileSize();
    const QSize mapSize = map->size();
    const qsizetype height = static_cast<qsizetype>(mapSize.height());
    const qsizetype width = static_cast<qsizetype>(mapSize.width());

    auto tileColor = [&](const QPoint& cell, const Tile& tile) {
        QColor color = Qt::gray;
        if (tile.type == TileType::Brick)
            color = QColor(193, 68, 14);
        if (tile.type == TileType::Steel)
            color = QColor(160, 160, 160);
        if (tile.type == TileType::Base) {
            const bool isBaseCell = (base && cell == baseCell);
            if (isBaseCell && baseDestroyed) {
                color = QColor(60, 60, 60);
            } else {
                color = (blinkPhase && isBaseCell) ? QColor(220, 40, 40) : QColor(230, 230, 0);
            }
        }
        return color;
    };

    for (qsizetype y = 0; y < height; ++y) {
        for (qsizetype x = 0; x < width; ++x) {
            const QPoint cell(static_cast<int>(x), static_cast<int>(y));
            const Tile tile = map->tile(cell);
            if (tile.type == TileType::Empty)
                continue;

            if (m_tileItems.contains(cell))
                continue;

            const QColor color = tileColor(cell, tile);
            const QPointF pos(static_cast<qreal>(x) * size, static_cast<qreal>(y) * size);
            QGraphicsRectItem* item = m_scene->addRect(QRectF(pos, QSizeF(size, size)), QPen(Qt::NoPen), QBrush(color));
#ifdef QT_DEBUG
            Q_ASSERT(!m_deletedItemsDebug.contains(item));
#endif
            item->setZValue(0);
            m_tileItems.insert(cell, item);
        }
    }
}

void Renderer::syncMapTiles(const Game& game)
{
    const Map* map = game.map();
    if (!map)
        return;
#ifdef QT_DEBUG
    auto debugMarkDeleted = [&](const QGraphicsItem* it) {
        Q_ASSERT(it);
        Q_ASSERT(!m_deletedItemsDebug.contains(it));
        m_deletedItemsDebug.insert(it);
    };
#endif

    const Base* base = game.base();
    const QPoint baseCell = base ? base->cell() : QPoint(-1, -1);
    const bool baseDestroyed = base && base->isDestroyed();
    const bool blinkPhase = m_baseBlinking && ((m_baseBlinkCounter / 8) % 2 == 0);

    auto tileColor = [&](const QPoint& cell, const Tile& tile) {
        QColor color = Qt::gray;
        if (tile.type == TileType::Brick)
            color = QColor(193, 68, 14);
        if (tile.type == TileType::Steel)
            color = QColor(160, 160, 160);
        if (tile.type == TileType::Base) {
            const bool isBaseCell = (base && cell == baseCell);
            if (isBaseCell && baseDestroyed) {
                color = QColor(60, 60, 60);
            } else {
                color = (blinkPhase && isBaseCell) ? QColor(220, 40, 40) : QColor(230, 230, 0);
            }
        }
        return color;
    };

    for (auto it = m_tileItems.begin(); it != m_tileItems.end();) {
        const QPoint cell = it.key();
        QGraphicsRectItem* item = it.value();
        const Tile tile = map->tile(cell);

        if (tile.type == TileType::Empty) {
            it = m_tileItems.erase(it);
            m_scene->removeItem(item);
#ifdef QT_DEBUG
            debugMarkDeleted(item);
#endif
            delete item;
            continue;
        }

        const QColor color = tileColor(cell, tile);
        if (item->brush().color() != color)
            item->setBrush(color);

        ++it;
    }
}

void Renderer::syncTanks(const Game& game)
{
    const qreal size = tileSize();
    const qreal barrelLength = size * 0.6;
    const qreal barrelThickness = size * 0.2;
    const QList<Tank*> tanks = game.tanks();
    QSet<const Tank*> seen;
    seen.reserve(tanks.size());
    for (Tank* tank : tanks) {
        if (tank)
            seen.insert(tank);
    }
#ifdef QT_DEBUG
    auto debugMarkDeleted = [&](const QGraphicsItem* it) {
        Q_ASSERT(it);
        Q_ASSERT(!m_deletedItemsDebug.contains(it));
        m_deletedItemsDebug.insert(it);
    };
#endif

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

    auto addExplosionAtItem = [&](QGraphicsRectItem* item) {
        if (!item || qFuzzyIsNull(size))
            return;

        const QPointF pos = item->pos();
        const QPoint cell(qRound(pos.x() / size), qRound(pos.y() / size));
        m_explosions.append(Explosion{cell, 12});
    };

    auto removeTankGraphics = [&](const Tank* tankPtr) {
        QGraphicsRectItem* item = m_tankItems.take(tankPtr);
        QGraphicsRectItem* directionItem = m_tankDirectionItems.take(tankPtr);

        if (!item && !directionItem)
            return;

#ifdef QT_DEBUG
        qDebug() << "Renderer removing tank visuals:" << tankPtr;
#endif

        addExplosionAtItem(item ? item : directionItem);

        if (item) {
            m_scene->removeItem(item);
#ifdef QT_DEBUG
            debugMarkDeleted(item);
#endif
            delete item;
        }

        if (directionItem) {
            m_scene->removeItem(directionItem);
#ifdef QT_DEBUG
            debugMarkDeleted(directionItem);
#endif
            delete directionItem;
        }
    };

    for (Tank* tank : tanks) {
        if (!tank)
            continue;

#ifdef QT_DEBUG
        Q_ASSERT(Tank::s_aliveTanks.contains(tank));
#endif
        QGraphicsRectItem* item = m_tankItems.value(tank, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(size, size)), QPen(Qt::black), QBrush(QColor(40, 160, 32)));
#ifdef QT_DEBUG
            Q_ASSERT(!m_deletedItemsDebug.contains(item));
#endif
            item->setZValue(10);
            m_tankItems.insert(tank, item);
        }

        QGraphicsRectItem* directionItem = m_tankDirectionItems.value(tank, nullptr);
        if (!directionItem) {
            directionItem = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(barrelThickness, barrelLength)), QPen(Qt::NoPen), QBrush(Qt::black));
#ifdef QT_DEBUG
            Q_ASSERT(!m_deletedItemsDebug.contains(directionItem));
#endif
            directionItem->setZValue(11);
            m_tankDirectionItems.insert(tank, directionItem);
        }

        QColor bodyColor(40, 160, 32);
        if (auto enemy = dynamic_cast<EnemyTank*>(tank)) {
            if (enemy->isHitFeedbackActive())
                bodyColor = QColor(230, 230, 230);
        }
        if (item->brush().color() != bodyColor)
            item->setBrush(bodyColor);

        const QPointF pos = QPointF(tank->cell()) * size;
        item->setPos(pos);
        directionItem->setRect(barrelRectForDirection(tank->direction()));
        directionItem->setPos(pos);
    }

    QSet<const Tank*> trackedTanks;
    trackedTanks.reserve(m_tankItems.size() + m_tankDirectionItems.size());

    for (auto it = m_tankItems.cbegin(); it != m_tankItems.cend(); ++it)
        trackedTanks.insert(it.key());

    for (auto it = m_tankDirectionItems.cbegin(); it != m_tankDirectionItems.cend(); ++it)
        trackedTanks.insert(it.key());

    for (const Tank* trackedTank : std::as_const(trackedTanks)) {
        if (!seen.contains(trackedTank))
            removeTankGraphics(trackedTank);
    }
}

void Renderer::syncBullets(const Game& game)
{
    const Map* map = game.map();
    const qreal size = tileSize();
    const qreal bulletSize = size / 2.0;
#ifdef QT_DEBUG
    auto debugMarkDeleted = [&](const QGraphicsItem* it) {
        Q_ASSERT(it);
        Q_ASSERT(!m_deletedItemsDebug.contains(it));
        m_deletedItemsDebug.insert(it);
    };
#endif
    QSet<QPoint> currentBulletCells;
    QHash<QPoint, bool> currentExplosionFlags;
    QHash<QPoint, QGraphicsRectItem*> newBulletItems;
    QSet<QPoint> reusedPreviousCells;

    const QPointF bulletOffset((size - bulletSize) / 2.0, (size - bulletSize) / 2.0);

    for (Bullet* bullet : game.bullets()) {
        if (!bullet || !bullet->isAlive())
            continue;

#ifdef QT_DEBUG
        Q_ASSERT(Bullet::s_aliveBullets.contains(bullet));
#endif

        const QPoint cell = bullet->cell();
        currentBulletCells.insert(cell);
        currentExplosionFlags.insert(cell, bullet->spawnExplosionOnDestroy());

        QGraphicsRectItem* item = m_bulletItems.take(cell);
        if (!item) {
            const QPoint previousCell = cell - bullet->directionDelta();
            item = m_bulletItems.take(previousCell);
            if (item)
                reusedPreviousCells.insert(previousCell);
        }

        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(bulletSize, bulletSize)), QPen(Qt::NoPen), QBrush(Qt::yellow));
#ifdef QT_DEBUG
            Q_ASSERT(!m_deletedItemsDebug.contains(item));
#endif
            item->setZValue(20);
        }

        const QPointF pos = QPointF(cell) * size + bulletOffset;
        item->setPos(pos);
        newBulletItems.insert(cell, item);
    }

    for (const QPoint& cell : m_previousBulletCells) {
        if (!currentBulletCells.contains(cell) && !reusedPreviousCells.contains(cell)) {
            const bool shouldExplode = m_previousBulletExplosionFlags.value(cell, true);
            if (map && map->isInside(cell) && shouldExplode)
                m_explosions.append(Explosion{cell, 12});
        }
    }

    for (QGraphicsRectItem* item : std::as_const(m_bulletItems)) {
        m_scene->removeItem(item);
#ifdef QT_DEBUG
        debugMarkDeleted(item);
#endif
        delete item;
    }

    m_bulletItems = newBulletItems;
    m_previousBulletCells = currentBulletCells;
    m_previousBulletExplosionFlags = currentExplosionFlags;
}

void Renderer::updateHud(const Game& game)
{
    if (!m_scene)
        return;

    const Map* map = game.map();
    if (!map)
        return;

    const qreal size = tileSize();
    const qreal hudMargin = size * 0.5;
    const qreal mapWidthInPixels = static_cast<qreal>(map->size().width()) * size;
    const QPointF hudPosition(mapWidthInPixels + hudMargin, hudMargin);

    auto ensureHudItem = [&](QGraphicsTextItem*& item, const QColor& color) {
        if (!item) {
            item = m_scene->addText(QString());
            item->setDefaultTextColor(color);
#ifdef QT_DEBUG
            Q_ASSERT(!m_deletedItemsDebug.contains(item));
#endif
            QFont font = item->font();
            font.setPointSize(16);
            item->setFont(font);
            item->setZValue(100);
        }
    };

    const int lives = game.state().remainingLives();
    const int enemyCount = game.state().aliveEnemies();

    ensureHudItem(m_hudLivesItem, QColor(80, 200, 120));
    ensureHudItem(m_hudEnemiesItem, QColor(220, 70, 70));

    const QString livesText = QStringLiteral("LIVES: %1").arg(lives);
    const QString enemiesText = QStringLiteral("ENEMIES: %1").arg(enemyCount);

    if (m_hudLivesItem->toPlainText() != livesText)
        m_hudLivesItem->setPlainText(livesText);

    if (m_hudEnemiesItem->toPlainText() != enemiesText)
        m_hudEnemiesItem->setPlainText(enemiesText);

    const qreal lineSpacing = size * 0.2;
    const qreal lineHeight = m_hudLivesItem->boundingRect().height();

    if (m_hudLivesItem->pos() != hudPosition)
        m_hudLivesItem->setPos(hudPosition);

    const QPointF enemiesPos = hudPosition + QPointF(0, lineHeight + lineSpacing);
    if (m_hudEnemiesItem->pos() != enemiesPos)
        m_hudEnemiesItem->setPos(enemiesPos);
}

void Renderer::updateBaseBlinking(const Game& game)
{
    const Base* base = game.base();
    const Map* map = game.map();

    if (!base || !map) {
        m_baseBlinking = false;
        m_baseBlinkCounter = 0;
        m_lastBaseHealth = -1;
        return;
    }

    const QPoint baseCell = base->cell();
    const bool baseTileExists = map->isInside(baseCell) && map->tile(baseCell).type == TileType::Base;
    const int currentHealth = base->health();
    const bool baseDestroyed = game.state().isBaseDestroyed() || base->isDestroyed();

    if (!baseTileExists || baseDestroyed) {
        m_baseBlinking = false;
        m_baseBlinkCounter = 0;
        m_lastBaseHealth = currentHealth;
        return;
    }

    if (m_lastBaseHealth != -1 && currentHealth < m_lastBaseHealth)
        m_baseBlinking = true;

    if (!m_baseBlinking) {
        m_baseBlinkCounter = 0;
    } else {
        ++m_baseBlinkCounter;
    }

    m_lastBaseHealth = currentHealth;
}

void Renderer::initializeMap(const Game& game)
{
    const Map* map = game.map();
    if (m_cachedMap == map)
        return;

#ifdef QT_DEBUG
    auto debugMarkDeleted = [&](const QGraphicsItem* it) {
        Q_ASSERT(it);
        Q_ASSERT(!m_deletedItemsDebug.contains(it));
        m_deletedItemsDebug.insert(it);
    };
#endif

    for (QGraphicsRectItem* item : std::as_const(m_tileItems)) {
        m_scene->removeItem(item);
#ifdef QT_DEBUG
        debugMarkDeleted(item);
#endif
        delete item;
    }
    m_tileItems.clear();

    m_cachedMap = map;
    drawMap(game);
}

qreal Renderer::tileSize() const
{
    return static_cast<qreal>(m_camera ? m_camera->tileSize() : TILE_SIZE);
}

void Renderer::updateExplosions()
{
#ifdef QT_DEBUG
    auto debugMarkDeleted = [&](const QGraphicsItem* it) {
        Q_ASSERT(it);
        Q_ASSERT(!m_deletedItemsDebug.contains(it));
        m_deletedItemsDebug.insert(it);
    };
#endif
    for (Explosion& explosion : m_explosions)
        --explosion.ttlFrames;

    auto it = std::remove_if(m_explosions.begin(), m_explosions.end(), [](const Explosion& e) { return e.ttlFrames <= 0; });
    m_explosions.erase(it, m_explosions.end());

    for (QGraphicsRectItem* item : m_explosionItems) {
        m_scene->removeItem(item);
#ifdef QT_DEBUG
        debugMarkDeleted(item);
#endif
        delete item;
    }
    m_explosionItems.clear();

    const qreal size = tileSize();
    const qreal explosionSize = size * 0.7;
    const QPointF offset((size - explosionSize) / 2.0, (size - explosionSize) / 2.0);
    const QBrush brush(QColor(255, 140, 0));

    for (const Explosion& explosion : std::as_const(m_explosions)) {
        const QPointF pos = QPointF(explosion.cell) * size + offset;
        QGraphicsRectItem* item = m_scene->addRect(QRectF(pos, QSizeF(explosionSize, explosionSize)), QPen(Qt::NoPen), brush);
#ifdef QT_DEBUG
        Q_ASSERT(!m_deletedItemsDebug.contains(item));
#endif
        item->setZValue(25);
        m_explosionItems.append(item);
    }
}
