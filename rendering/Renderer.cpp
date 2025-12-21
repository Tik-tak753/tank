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
#include <algorithm>
#include <utility>
#include <QString>

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

namespace {
const QColor kHudLabelColor(210, 210, 210);
const QColor kHudLivesColor(230, 190, 60);
const QColor kHudEnemyColor(210, 70, 70);
const QColor kHudStatusColor(200, 200, 200);

QString toCssColor(const QColor& color)
{
    return color.name(QColor::HexRgb);
}
} // namespace

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
    clearMapLayer();     // DEBUG ONLY
    drawMap(game);       // reflect runtime tile changes

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

    for (qsizetype y = 0; y < height; ++y) {
        for (qsizetype x = 0; x < width; ++x) {
            const QPoint cell(static_cast<int>(x), static_cast<int>(y));
            const Tile tile = map->tile(cell);
            if (tile.type == TileType::Empty)
                continue;

            QColor color = Qt::gray;
            qreal zValue = 0;
            if (tile.type == TileType::Brick)
                color = QColor(193, 68, 14);
            if (tile.type == TileType::Steel)
                color = QColor(160, 160, 160);
            if (tile.type == TileType::Water)
                color = QColor(60, 120, 200);
            if (tile.type == TileType::Ice) {
                color = QColor(210, 230, 240);
                zValue = 5;
            }
            if (tile.type == TileType::Forest) {
                color = QColor(50, 120, 60, 210);
                zValue = 15;
            }
            if (tile.type == TileType::Base) {
                const bool isBaseCell = (base && cell == baseCell);
                if (isBaseCell && baseDestroyed) {
                    color = QColor(60, 60, 60);
                } else {
                    color = (blinkPhase && isBaseCell) ? QColor(220, 40, 40) : QColor(230, 230, 0);
                }
            }

            const QPointF pos(static_cast<qreal>(x) * size, static_cast<qreal>(y) * size);
            QGraphicsRectItem* item = m_scene->addRect(QRectF(pos, QSizeF(size, size)), QPen(Qt::NoPen), QBrush(color));
            item->setZValue(zValue);
            m_mapItems.append(item);

            if (tile.type == TileType::Base && baseDestroyed && cell == baseCell) {
                const qreal markerMargin = size * 0.25;
                const QRectF markerRect(pos + QPointF(markerMargin, markerMargin), QSizeF(size - 2 * markerMargin, size - 2 * markerMargin));
                QGraphicsRectItem* marker = m_scene->addRect(markerRect, QPen(Qt::NoPen), QBrush(QColor(30, 30, 30)));
                marker->setZValue(1);
                m_mapItems.append(marker);
            }
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

        if (tank->isDestroyed()) {
            if (!m_destroyedTanks.contains(tank))
                m_explosions.append(Explosion{tank->cell(), 12});

            m_destroyedTanks.insert(tank);

            auto removeItemSafely = [&](QGraphicsItem* item, const QString& reason) {
                if (!item)
                    return;
                m_scene->removeItem(item);
                delete item;
            };

            QGraphicsRectItem* item = m_tankItems.take(tank);
            removeItemSafely(item, "Tank destroy");

            QGraphicsRectItem* directionItem = m_tankDirectionItems.take(tank);
            removeItemSafely(directionItem, "Tank destroy");

            continue;
        } else {
            m_destroyedTanks.remove(tank);
        }
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

        QColor bodyColor(40, 160, 32);
        if (tank->getType() == TankType::Player) {
            bodyColor = QColor(230, 190, 60);
        } else if (auto enemy = dynamic_cast<EnemyTank*>(tank)) {
            if (enemy->isHitFeedbackActive())
                bodyColor = QColor(230, 230, 230);
            else
                bodyColor = QColor(100, 120, 180);
        }
        if (item->brush().color() != bodyColor)
            item->setBrush(bodyColor);

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

    auto destroyedIt = m_destroyedTanks.begin();
    while (destroyedIt != m_destroyedTanks.end()) {
        if (!seen.contains(*destroyedIt))
            destroyedIt = m_destroyedTanks.erase(destroyedIt);
        else
            ++destroyedIt;
    }
}

void Renderer::syncBullets(const Game& game)
{
    const Map* map = game.map();
    const qreal size = tileSize();
    const qreal bulletSize = size / 2.0;
    QSet<const Bullet*> currentBullets;

    const QPointF bulletOffset((size - bulletSize) / 2.0, (size - bulletSize) / 2.0);

    for (Bullet* bullet : game.bullets()) {
        if (!bullet)
            continue;

        if (!bullet->isAlive()) {
            m_lastBulletCells.insert(bullet, bullet->cell());
            m_lastBulletExplosions.insert(bullet, bullet->spawnExplosionOnDestroy());
            continue;
        }

        currentBullets.insert(bullet);
        QGraphicsRectItem* item = m_bulletItems.value(bullet, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(bulletSize, bulletSize)), QPen(Qt::NoPen), QBrush(Qt::yellow));
            item->setZValue(20);
            m_bulletItems.insert(bullet, item);
        }

        const QPointF pos = QPointF(bullet->cell()) * size + bulletOffset;
        item->setPos(pos);
        m_lastBulletCells.insert(bullet, bullet->cell());
        m_lastBulletExplosions.insert(bullet, bullet->spawnExplosionOnDestroy());
    }

    for (const Bullet* bullet : m_previousBullets) {
        if (!currentBullets.contains(bullet)) {
            const QPoint cell = m_lastBulletCells.value(bullet, QPoint(-1, -1));
            const bool shouldExplode = m_lastBulletExplosions.value(bullet, true);
            if (map && map->isInside(cell) && shouldExplode)
                m_explosions.append(Explosion{cell, 12});
            m_lastBulletCells.remove(bullet);
            m_lastBulletExplosions.remove(bullet);
        }
    }

    auto it = m_bulletItems.begin();
    while (it != m_bulletItems.end()) {
        if (!currentBullets.contains(it.key())) {
            QGraphicsItem* item = it.value();
            m_scene->removeItem(item);
            delete item;
            it = m_bulletItems.erase(it);
        } else {
            ++it;
        }
    }

    auto cellIt = m_lastBulletCells.begin();
    while (cellIt != m_lastBulletCells.end()) {
        if (!currentBullets.contains(cellIt.key()))
            cellIt = m_lastBulletCells.erase(cellIt);
        else
            ++cellIt;
    }

    auto explodeIt = m_lastBulletExplosions.begin();
    while (explodeIt != m_lastBulletExplosions.end()) {
        if (!currentBullets.contains(explodeIt.key()))
            explodeIt = m_lastBulletExplosions.erase(explodeIt);
        else
            ++explodeIt;
    }

    m_previousBullets = currentBullets;
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

    if (!m_hudItem) {
        m_hudItem = m_scene->addText(QString());
        m_hudItem->setDefaultTextColor(kHudLabelColor);
        QFont font = m_hudItem->font();
        font.setPointSize(16);
        m_hudItem->setFont(font);
        m_hudItem->setZValue(100);
    }

    if (m_hudItem->pos() != hudPosition)
        m_hudItem->setPos(hudPosition);

    const int lives = game.state().remainingLives();
    const int enemyCount = game.state().aliveEnemies();
    QString statusText;
    switch (game.state().sessionState()) {
    case GameSessionState::Running:
        break;
    case GameSessionState::GameOver:
        statusText = QStringLiteral("GAME OVER");
        break;
    case GameSessionState::Victory:
        statusText = QStringLiteral("STAGE CLEAR");
        break;
    }

    const QString text = QStringLiteral(
                             "<div style='color:%1;'>"
                             "<span style='color:%2;'>LIVES:</span> <span style='color:%3;'>%4</span><br/>"
                             "<span style='color:%2;'>ENEMIES:</span> <span style='color:%5;'>%6</span>%7"
                             "</div>")
                             .arg(toCssColor(kHudLabelColor))
                             .arg(toCssColor(kHudLabelColor))
                             .arg(toCssColor(kHudLivesColor))
                             .arg(lives)
                             .arg(toCssColor(kHudEnemyColor))
                             .arg(enemyCount)
                             .arg(statusText.isEmpty() ? QString() : QStringLiteral("<br/><span style='color:%1;'>%2</span>")
                                                                                       .arg(toCssColor(kHudStatusColor))
                                                                                       .arg(statusText));

    if (m_hudItem->toHtml() != text)
        m_hudItem->setHtml(text);

    if (m_lastHudStatus != statusText) {
        m_lastHudStatus = statusText;
    }
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

void Renderer::updateExplosions()
{
    for (Explosion& explosion : m_explosions)
        --explosion.ttlFrames;

    auto it = std::remove_if(m_explosions.begin(), m_explosions.end(), [](const Explosion& e) { return e.ttlFrames <= 0; });
    m_explosions.erase(it, m_explosions.end());

    for (QGraphicsRectItem* item : m_explosionItems) {
        m_scene->removeItem(item);
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
        item->setZValue(25);
        m_explosionItems.append(item);
    }
}
