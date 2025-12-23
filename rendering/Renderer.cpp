#include "rendering/Renderer.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <QSet>
#include <QtGlobal>
#include <cmath>
#include <algorithm>
#include <functional>
#include <utility>
#include <QString>

#include "core/Game.h"
#include "gameplay/Bullet.h"
#include "gameplay/Tank.h"
#include "gameplay/EnemyTank.h"
#include "gameplay/PlayerTank.h"
#include "gameplay/Bonus.h"
#include "rendering/Camera.h"
#include "rendering/SpriteManager.h"
#include "rendering/HudItem.h"
#include "utils/Constants.h"
#include "world/Base.h"
#include "world/Map.h"
#include "world/Tile.h"

namespace {
constexpr int kExplosionFrameCount = 3;
constexpr qreal kExplosionMinScale = 0.35;
constexpr qreal kExplosionMaxScale = 0.95;
constexpr qreal kPi = 3.14159265358979323846;
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

void Renderer::renderFrame(const Game& game, qreal alpha)
{
    if (!m_scene)
        return;

    updateRenderTransform(game);
    updateBaseBlinking(game);
    updateBackground(game);
    clearMapLayer();     // DEBUG ONLY
    drawMap(game);       // reflect runtime tile changes

    syncBonuses(game);
    syncTanks(game, alpha);
    syncBullets(game, alpha);
    updateExplosions();
    updateHud(game);
}

void Renderer::updateRenderTransform(const Game& game)
{
    const Map* map = game.map();
    if (!m_scene || !map)
        return;

    QGraphicsView* view = m_scene->views().isEmpty() ? nullptr : m_scene->views().first();
    if (!view || !view->viewport())
        return;

    const QSize viewportSize = view->viewport()->size();
    if (viewportSize.isEmpty())
        return;

    const QSize mapSize = map->size();
    if (mapSize.isEmpty())
        return;

    const qreal viewportWidth = static_cast<qreal>(viewportSize.width());
    const qreal viewportHeight = static_cast<qreal>(viewportSize.height());
    const qreal mapWidthTiles = static_cast<qreal>(mapSize.width());
    const qreal mapHeightTiles = static_cast<qreal>(mapSize.height());

    const qreal scale = std::min(viewportWidth / mapWidthTiles, viewportHeight / mapHeightTiles);
    if (scale <= 0.0)
        return;

    m_tileScale = scale;

    const qreal mapWidthInPixels = mapWidthTiles * m_tileScale;
    const qreal mapHeightInPixels = mapHeightTiles * m_tileScale;
    m_renderOffset = QPointF((viewportWidth - mapWidthInPixels) / 2.0,
                             (viewportHeight - mapHeightInPixels) / 2.0);

    if (m_camera)
        m_camera->setTileSize(m_tileScale);

    m_scene->setSceneRect(QRectF(QPointF(0.0, 0.0), QSizeF(viewportSize)));
}

void Renderer::updateBackground(const Game& game)
{
    if (!m_scene)
        return;

    static bool backgroundInitialized = false;
    if (!backgroundInitialized) {
        QPixmap pattern(24, 24);
        pattern.fill(QColor(12, 12, 16));
        QPainter painter(&pattern);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(QColor(22, 22, 28));
        painter.drawRect(pattern.rect().adjusted(0, 0, -1, -1));
        painter.drawLine(0, pattern.height() / 2, pattern.width(), pattern.height() / 2);
        painter.drawLine(pattern.width() / 2, 0, pattern.width() / 2, pattern.height());
        painter.end();
        m_scene->setBackgroundBrush(QBrush(pattern));
        backgroundInitialized = true;
    }

    const Map* map = game.map();
    if (!map)
        return;

    const QSize mapSize = map->size();
    if (mapSize.isEmpty())
        return;

    const qreal size = tileSize();
    const qreal mapWidthInPixels = static_cast<qreal>(mapSize.width()) * size;
    const qreal mapHeightInPixels = static_cast<qreal>(mapSize.height()) * size;
    const QRectF mapRect(m_renderOffset, QSizeF(mapWidthInPixels, mapHeightInPixels));

    const qreal frameWidth = std::max(1.0, size * 0.06);
    const QPen framePen(QColor(70, 70, 80), frameWidth);

    if (!m_mapFrameItem) {
        m_mapFrameItem = m_scene->addRect(mapRect, framePen, Qt::NoBrush);
        m_mapFrameItem->setZValue(-10);
        m_mapFrameItem->setAcceptedMouseButtons(Qt::NoButton);
        m_mapFrameItem->setAcceptHoverEvents(false);
        m_mapFrameItem->setFlag(QGraphicsItem::ItemIsFocusable, false);
    } else {
        m_mapFrameItem->setRect(mapRect);
        m_mapFrameItem->setPen(framePen);
    }
}

void Renderer::drawMap(const Game& game)
{
    const Map* map = game.map();
    if (!map)
        return;
    rebuildTileBrushes(tileSize());
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
            QBrush brush;
            qreal zValue = 0;
            if (tile.type == TileType::Brick) {
                color = QColor(193, 68, 14);
                brush = tileBrush(static_cast<int>(TileType::Brick), size);
            }
            if (tile.type == TileType::Steel) {
                color = QColor(160, 160, 160);
                brush = tileBrush(static_cast<int>(TileType::Steel), size);
            }
            if (tile.type == TileType::Water) {
                color = QColor(60, 120, 200);
                brush = tileBrush(static_cast<int>(TileType::Water), size);
                zValue = 2;
            }
            if (tile.type == TileType::Ice) {
                color = QColor(210, 230, 240);
                zValue = 5;
            }
            if (tile.type == TileType::Forest) {
                color = QColor(50, 120, 60, 210);
                brush = tileBrush(static_cast<int>(TileType::Forest), size);
                zValue = 15;
            }
            if (tile.type == TileType::Base) {
                const bool isBaseCell = (base && cell == baseCell);
                const bool destroyedBaseTile = isBaseCell && baseDestroyed;
                const bool blinkingBase = isBaseCell && blinkPhase;
                color = destroyedBaseTile ? QColor(60, 60, 60) : QColor(230, 230, 0);
                brush = baseTileBrush(destroyedBaseTile, blinkingBase, size);
            }

            const QPointF pos = cellToScene(cell);
            QBrush fillBrush = brush;
            if (fillBrush.style() == Qt::NoBrush && fillBrush.texture().isNull())
                fillBrush = QBrush(color);
            QGraphicsRectItem* item = m_scene->addRect(QRectF(pos, QSizeF(size, size)), QPen(Qt::NoPen), fillBrush);
            item->setZValue(zValue);
            m_mapItems.append(item);
        }
    }
}

QBrush Renderer::tileBrush(int tileType, qreal size)
{
    rebuildTileBrushes(size);
    return m_tileBrushes.value(tileType, QBrush());
}

QBrush Renderer::baseTileBrush(bool destroyed, bool blinkPhase, qreal size)
{
    const int intSize = qMax(1, qRound(size));
    const quint64 key = (static_cast<quint64>(intSize) << 2)
                        | (destroyed ? 0x1 : 0x0)
                        | (blinkPhase ? 0x2 : 0x0);
    static QHash<quint64, QBrush> cache;
    const auto cached = cache.constFind(key);
    if (cached != cache.constEnd())
        return cached.value();

    QPixmap pixmap(intSize, intSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);

    const QColor borderColor = destroyed ? QColor(40, 40, 40) : QColor(100, 80, 20);
    const QColor plateColor = destroyed ? QColor(65, 65, 65) : (blinkPhase ? QColor(205, 40, 40) : QColor(230, 210, 60));
    const QColor emblemColor = destroyed ? QColor(180, 180, 180) : QColor(30, 30, 30);
    const QColor accentColor = destroyed ? QColor(110, 110, 110) : QColor(245, 235, 170);

    const int frame = qMax(1, intSize / 12);
    const int innerSize = intSize - 2 * frame;
    painter.fillRect(0, 0, intSize, intSize, borderColor);
    painter.fillRect(frame, frame, innerSize, innerSize, plateColor);

    const int margin = qMax(1, intSize / 10);
    const int wingHeight = qMax(2, intSize / 8);
    const int bodyWidth = qMax(2, intSize / 9);
    const int emblemTop = frame + margin;
    const int emblemBottom = intSize - frame - margin;
    const int centerX = intSize / 2;

    painter.fillRect(frame + margin, emblemTop + wingHeight / 2, intSize - 2 * (frame + margin), wingHeight, emblemColor);

    painter.fillRect(centerX - bodyWidth / 2, emblemTop, bodyWidth, emblemBottom - emblemTop, emblemColor);

    const int headSize = qMax(2, intSize / 12);
    painter.fillRect(centerX - headSize / 2, emblemTop - wingHeight / 3, headSize, wingHeight, accentColor);

    const int tailWidth = qMax(bodyWidth * 2, intSize / 5);
    const int tailHeight = qMax(2, intSize / 10);
    painter.fillRect(centerX - tailWidth / 2, emblemBottom - tailHeight, tailWidth, tailHeight, emblemColor);

    const int clawWidth = qMax(2, intSize / 14);
    const int clawHeight = qMax(2, tailHeight / 2);
    painter.fillRect(centerX - tailWidth / 2, emblemBottom - clawHeight, clawWidth, clawHeight, accentColor);
    painter.fillRect(centerX + tailWidth / 2 - clawWidth, emblemBottom - clawHeight, clawWidth, clawHeight, accentColor);

    const int coreWidth = qMax(2, bodyWidth / 2);
    const int coreHeight = qMax(2, intSize / 12);
    painter.fillRect(centerX - coreWidth / 2, emblemTop + wingHeight + coreHeight / 2, coreWidth, coreHeight, accentColor);

    if (destroyed) {
        const QColor crackColor(20, 20, 20);
        const int penWidth = qMax(1, intSize / 18);
        painter.setPen(QPen(crackColor, penWidth));
        painter.drawLine(QPoint(frame + margin, emblemTop + wingHeight),
                         QPoint(intSize - frame - margin, emblemBottom - tailHeight / 2));
        painter.drawLine(QPoint(centerX - bodyWidth, emblemTop),
                         QPoint(centerX + margin / 2, emblemBottom));
        painter.setPen(Qt::NoPen);
    }

    painter.end();

    QBrush brush(pixmap);
    cache.insert(key, brush);
    return brush;
}

void Renderer::rebuildTileBrushes(qreal size)
{
    const int intSize = qMax(1, qRound(size));
    if (m_tileBrushSize == intSize)
        return;

    m_tileBrushSize = intSize;
    m_tileBrushes.clear();

    auto makeBrush = [&](const std::function<void(QPainter&, int)>& drawer) {
        QPixmap pixmap(intSize, intSize);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);
        drawer(painter, intSize);
        painter.end();
        return QBrush(pixmap);
    };

    m_tileBrushes.insert(static_cast<int>(TileType::Brick), makeBrush([](QPainter& painter, int s) {
        const QColor mortar(130, 80, 60);
        const QColor brick(193, 68, 14);
        const QColor highlight = brick.lighter(130);
        const QColor shadow = brick.darker(120);

        painter.fillRect(0, 0, s, s, mortar);

        const int gap = qMax(1, s / 12);
        const int brickW = (s - gap) / 2;
        const int brickH = (s - gap) / 2;
        const int edge = qMax(1, brickH / 6);

        for (int row = 0; row < 2; ++row) {
            for (int col = 0; col < 2; ++col) {
                const int x = col * (brickW + gap);
                const int y = row * (brickH + gap);
                QRect rect(x, y, brickW, brickH);
                painter.fillRect(rect, brick);
                painter.fillRect(QRect(rect.x(), rect.y(), rect.width(), edge), highlight);
                painter.fillRect(QRect(rect.x(), rect.y() + rect.height() - edge, rect.width(), edge), shadow);
            }
        }
    }));

    m_tileBrushes.insert(static_cast<int>(TileType::Steel), makeBrush([](QPainter& painter, int s) {
        const QColor border(90, 90, 100);
        const QColor center(185, 185, 195);
        const QColor groove(70, 70, 80);

        const int borderSize = qMax(1, s / 10);
        const int inset = borderSize * 2;
        const int grooveWidth = qMax(1, s / 16);

        painter.fillRect(0, 0, s, s, border);
        painter.fillRect(borderSize, borderSize, s - 2 * borderSize, s - 2 * borderSize, center);
        painter.fillRect(inset, s / 2 - grooveWidth / 2, s - 2 * inset, grooveWidth, groove);
        painter.fillRect(s / 2 - grooveWidth / 2, inset, grooveWidth, s - 2 * inset, groove);
    }));

    m_tileBrushes.insert(static_cast<int>(TileType::Water), makeBrush([](QPainter& painter, int s) {
        const QColor deep(40, 100, 180);
        const QColor ripple(90, 160, 230);

        painter.fillRect(0, 0, s, s, deep);

        const int stripe = qMax(1, s / 8);
        for (int y = 0; y < s; y += stripe * 2)
            painter.fillRect(0, y, s, stripe, ripple);

        const int accent = qMax(1, stripe / 2);
        painter.fillRect(0, stripe / 2, s, accent, deep.lighter(115));
    }));

    m_tileBrushes.insert(static_cast<int>(TileType::Forest), makeBrush([](QPainter& painter, int s) {
        const QColor canopy(40, 120, 70, 140);
        const QColor leafA(60, 150, 80, 180);
        const QColor leafB(30, 90, 50, 160);

        painter.fillRect(0, 0, s, s, canopy);

        const int patch = qMax(2, s / 6);
        for (int y = 0; y < s; y += patch) {
            const int offset = (y / patch) % 2 ? patch / 2 : 0;
            for (int x = offset; x < s; x += patch) {
                const QColor color = ((x / patch + y / patch) % 2) ? leafA : leafB;
                painter.fillRect(x, y, patch, patch, color);
            }
        }
    }));
}

QBrush Renderer::bonusBrush(BonusType type, qreal size)
{
    const int intSize = qMax(1, qRound(size));
    const quint64 key = (static_cast<quint64>(intSize) << 32) | static_cast<quint64>(type);
    static QHash<quint64, QBrush> cache;
    const auto cached = cache.constFind(key);
    if (cached != cache.constEnd())
        return cached.value();

    const auto withShadow = [&](QPainter& painter, const QColor& shadowColor) {
        const int shadow = qMax(1, intSize / 18);
        painter.fillRect(shadow, shadow, intSize - shadow, intSize - shadow, shadowColor);
    };

    QPixmap pixmap(intSize, intSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    switch (type) {
    case BonusType::Star: {
        const QColor body(255, 220, 60);
        const QColor highlight(255, 245, 165);
        const QColor outline(140, 90, 15);
        const QColor shadow(90, 55, 10);
        withShadow(painter, shadow);

        const QPointF center(intSize / 2.0, intSize / 2.0);
        const qreal outerRadius = (intSize / 2.0) - qMax(1, intSize / 12);
        const qreal innerRadius = outerRadius * 0.45;
        QPolygonF polygon;
        for (int i = 0; i < 10; ++i) {
            const qreal angle = -kPi / 2.0 + i * kPi / 5.0;
            const qreal radius = (i % 2 == 0) ? outerRadius : innerRadius;
            polygon << QPointF(center.x() + std::cos(angle) * radius,
                                center.y() + std::sin(angle) * radius);
        }

        painter.setBrush(body);
        painter.setPen(QPen(outline, qMax(1, intSize / 18)));
        painter.drawPolygon(polygon);

        painter.setPen(Qt::NoPen);
        QPolygonF glow;
        const qreal glowOuter = outerRadius * 0.75;
        const qreal glowInner = innerRadius * 0.65;
        for (int i = 0; i < 10; ++i) {
            const qreal angle = -kPi / 2.0 + i * kPi / 5.0;
            const qreal radius = (i % 2 == 0) ? glowOuter : glowInner;
            glow << QPointF(center.x() + std::cos(angle) * radius,
                            center.y() + std::sin(angle) * radius);
        }
        painter.setBrush(highlight);
        painter.drawPolygon(glow);
        break;
    }
    case BonusType::Helmet: {
        const QColor shell(120, 200, 255);
        const QColor visor(35, 70, 105);
        const QColor outline(20, 60, 90);
        const QColor shine(200, 240, 255);
        withShadow(painter, QColor(15, 40, 60, 180));

        const int margin = qMax(1, intSize / 8);
        const int shellTop = margin + margin / 5;
        const QRect shellRect(margin, shellTop, intSize - 2 * margin, intSize - margin * 2);

        painter.setBrush(shell);
        painter.setPen(QPen(outline, qMax(1, intSize / 22)));
        painter.drawRoundedRect(shellRect, shellRect.width() / 6.0, shellRect.height() / 6.0);

        const int brimHeight = qMax(2, intSize / 7);
        painter.setPen(Qt::NoPen);
        painter.setBrush(outline);
        painter.drawRect(shellRect.x(), shellRect.bottom() - brimHeight, shellRect.width(), brimHeight);

        const int visorHeight = qMax(2, shellRect.height() / 3);
        const int visorWidth = shellRect.width() - qMax(2, intSize / 10);
        const int visorX = shellRect.x() + (shellRect.width() - visorWidth) / 2;
        const int visorY = shellRect.y() + shellRect.height() / 3;
        painter.setBrush(visor);
        painter.drawRoundedRect(QRect(visorX, visorY, visorWidth, visorHeight), visorHeight / 3.0, visorHeight / 3.0);

        painter.setBrush(shine);
        const int shineWidth = qMax(1, visorWidth / 3);
        painter.drawRect(QRect(visorX + visorWidth / 6, visorY + visorHeight / 4, shineWidth, visorHeight / 6));
        break;
    }
    case BonusType::Clock: {
        const QColor rim(255, 255, 255);
        const QColor face(235, 245, 255);
        const QColor tick(90, 120, 160);
        const QColor hand(200, 80, 80);
        withShadow(painter, QColor(25, 30, 55, 190));

        const int radius = intSize / 2 - qMax(1, intSize / 12);
        const QPoint center(intSize / 2, intSize / 2);
        painter.setBrush(rim);
        painter.setPen(QPen(tick.darker(140), qMax(1, intSize / 18)));
        painter.drawEllipse(center, radius, radius);

        painter.setBrush(face);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, radius - qMax(1, intSize / 14), radius - qMax(1, intSize / 14));

        painter.setPen(QPen(tick, qMax(1, intSize / 24)));
        const int tickLength = qMax(2, radius / 4);
        for (int i = 0; i < 12; ++i) {
            const qreal angle = -kPi / 2.0 + i * kPi / 6.0;
            const QPoint inner(center.x() + std::cos(angle) * (radius - tickLength),
                               center.y() + std::sin(angle) * (radius - tickLength));
            const QPoint outer(center.x() + std::cos(angle) * (radius - qMax(1, intSize / 20)),
                               center.y() + std::sin(angle) * (radius - qMax(1, intSize / 20)));
            painter.drawLine(inner, outer);
        }

        painter.setPen(QPen(hand, qMax(1, intSize / 18)));
        const QPoint hourHand(center.x() + std::cos(kPi / 3.0) * (radius * 0.45),
                              center.y() - std::sin(kPi / 3.0) * (radius * 0.45));
        painter.drawLine(center, hourHand);
        painter.setPen(QPen(tick, qMax(1, intSize / 20)));
        const QPoint minuteHand(center.x() + std::cos(-kPi / 6.0) * (radius * 0.65),
                                center.y() + std::sin(-kPi / 6.0) * (radius * 0.65));
        painter.drawLine(center, minuteHand);
        painter.setBrush(hand);
        painter.drawEllipse(center, qMax(1, intSize / 18), qMax(1, intSize / 18));
        break;
    }
    case BonusType::Grenade: {
        const QColor body(90, 160, 95);
        const QColor segments(65, 115, 75);
        const QColor pin(200, 200, 210);
        const QColor fuse(180, 130, 70);
        withShadow(painter, QColor(20, 40, 25, 180));

        const int margin = qMax(1, intSize / 8);
        QRect bodyRect(margin, margin, intSize - 2 * margin, intSize - 2 * margin);
        painter.setBrush(body);
        painter.setPen(QPen(segments.darker(120), qMax(1, intSize / 24)));
        painter.drawRoundedRect(bodyRect, bodyRect.width() / 7.0, bodyRect.width() / 7.0);

        painter.setPen(QPen(segments, qMax(1, intSize / 20)));
        const int rows = 3;
        const int cols = 2;
        for (int r = 1; r < rows; ++r) {
            const int y = bodyRect.y() + (bodyRect.height() * r) / rows;
            painter.drawLine(bodyRect.x() + bodyRect.width() / 8, y, bodyRect.right() - bodyRect.width() / 8, y);
        }
        for (int c = 1; c < cols; ++c) {
            const int x = bodyRect.x() + (bodyRect.width() * c) / cols;
            painter.drawLine(x, bodyRect.y() + bodyRect.height() / 6, x, bodyRect.bottom() - bodyRect.height() / 6);
        }

        painter.setPen(QPen(pin, qMax(1, intSize / 16)));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(bodyRect.adjusted(-margin / 2, -margin, margin / 2, -margin / 2), 30 * 16, 210 * 16);

        painter.setPen(QPen(fuse, qMax(1, intSize / 20)));
        const int fuseLength = qMax(2, intSize / 4);
        painter.drawLine(bodyRect.center(), QPoint(bodyRect.center().x(), bodyRect.y() - fuseLength / 2));
        break;
    }
    }

    painter.end();

    QBrush brush(pixmap);
    cache.insert(key, brush);
    return brush;
}

void Renderer::syncBonuses(const Game& game)
{
    if (!m_scene)
        return;

    const qreal size = tileSize();
    const qreal bonusSize = size * 0.6;
    const QPointF offset((size - bonusSize) / 2.0, (size - bonusSize) / 2.0);
    QSet<const Bonus*> seen;

    for (Bonus* bonus : game.bonuses()) {
        if (!bonus || bonus->isCollected())
            continue;

        seen.insert(bonus);
        QGraphicsRectItem* item = m_bonusItems.value(bonus, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), QSizeF(bonusSize, bonusSize)),
                                    QPen(Qt::NoPen), bonusBrush(bonus->type(), bonusSize));
            item->setZValue(8);
            m_bonusItems.insert(bonus, item);
        }

        const QBrush brush = bonusBrush(bonus->type(), bonusSize);
        if (item->brush() != brush)
            item->setBrush(brush);

        const QPointF pos = cellToScene(bonus->cell()) + offset;
        item->setPos(pos);
    }

    auto it = m_bonusItems.begin();
    while (it != m_bonusItems.end()) {
        if (!seen.contains(it.key())) {
            QGraphicsItem* item = it.value();
            m_scene->removeItem(item);
            delete item;
            it = m_bonusItems.erase(it);
        } else {
            ++it;
        }
    }
}

void Renderer::syncTanks(const Game& game, qreal alpha)
{
    const qreal size = tileSize();
    const qreal barrelLength = size * 0.65;
    const qreal barrelThickness = size * 0.16;
    const int intSize = qMax(1, qRound(size));
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

    auto tankBrushForColor = [intSize](const QColor& bodyColor) {
        static QHash<quint64, QBrush> cache;
        const quint64 key = (static_cast<quint64>(intSize) << 32) | static_cast<quint64>(bodyColor.rgba());
        const auto cached = cache.constFind(key);
        if (cached != cache.constEnd())
            return cached.value();

        const QColor trackColor = bodyColor.darker(190);
        const QColor trackHighlight = trackColor.lighter(135);
        const QColor bodyHighlight = bodyColor.lighter(125);
        const QColor bodyShadow = bodyColor.darker(160);
        const QColor turretColor = bodyColor.lighter(115);
        const QColor turretCore = turretColor.darker(135);
        const QColor topLayer = bodyColor.lighter(110);
        const QColor bottomLayer = bodyColor.darker(115);

        QPixmap pixmap(intSize, intSize);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);

        const int trackWidth = qMax(1, intSize / 6);
        const int bodyWidth = intSize - 2 * trackWidth;

        painter.fillRect(0, 0, trackWidth, intSize, trackColor);
        painter.fillRect(intSize - trackWidth, 0, trackWidth, intSize, trackColor);

        const int trackEdge = qMax(1, intSize / 16);
        painter.fillRect(0, 0, trackWidth, trackEdge, trackHighlight);
        painter.fillRect(intSize - trackWidth, intSize - trackEdge, trackWidth, trackEdge, trackHighlight);

        const int treadSegments = intSize >= 12 ? 3 : 2;
        const int treadHeight = qMax(1, intSize / (treadSegments * 2));
        for (int i = 0; i < treadSegments; ++i) {
            const int y = ((i + 1) * intSize) / (treadSegments + 1) - treadHeight / 2;
            painter.fillRect(0, y, trackWidth, treadHeight, trackHighlight);
            painter.fillRect(intSize - trackWidth, y, trackWidth, treadHeight, trackHighlight);
        }

        const int bodyMid = intSize / 2;
        painter.fillRect(trackWidth, 0, bodyWidth, bodyMid, topLayer);
        painter.fillRect(trackWidth, bodyMid, bodyWidth, intSize - bodyMid, bottomLayer);
        const int edge = qMax(1, intSize / 16);
        painter.fillRect(trackWidth, 0, bodyWidth, edge, bodyHighlight);
        painter.fillRect(trackWidth, intSize - edge, bodyWidth, edge, bodyShadow);
        painter.fillRect(trackWidth, edge, edge, intSize - 2 * edge, bodyHighlight);
        painter.fillRect(trackWidth + bodyWidth - edge, edge, edge, intSize - 2 * edge, bodyShadow);

        const int turretSize = qMax(intSize / 3, bodyWidth / 2);
        const int turretX = (intSize - turretSize) / 2;
        const int turretY = (intSize - turretSize) / 2;
        painter.fillRect(turretX, turretY, turretSize, turretSize, turretColor);

        const int turretInset = qMax(1, turretSize / 6);
        painter.fillRect(turretX + turretInset, turretY + turretInset,
                         turretSize - 2 * turretInset, turretInset, bodyHighlight);
        painter.fillRect(turretX + turretInset, turretY + turretSize - 2 * turretInset,
                         turretSize - 2 * turretInset, turretInset, bodyShadow);

        const int coreSize = qMax(1, turretSize / 3);
        const int coreX = turretX + (turretSize - coreSize) / 2;
        const int coreY = turretY + (turretSize - coreSize) / 2;
        painter.fillRect(coreX, coreY, coreSize, coreSize, turretCore);

        painter.end();

        QBrush brush(pixmap);
        cache.insert(key, brush);
        return brush;
    };

    auto playerColorForStars = [](int stars) {
        if (stars >= PlayerTank::maxStars())
            return QColor(255, 255, 170);
        if (stars >= 1)
            return QColor(245, 215, 110);
        return QColor(230, 190, 60);
    };

    for (Tank* tank : game.tanks()) {
        if (!tank)
            continue;

        seen.insert(tank);

        if (tank->isDestroyed()) {
            if (!m_destroyedTanks.contains(tank))
                m_explosions.append(Explosion{tank->cell(), kExplosionFrameCount, kExplosionFrameCount});

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
        if (tank == game.player()) {
            bodyColor = playerColorForStars(game.playerStars());
        } else if (auto enemy = dynamic_cast<EnemyTank*>(tank)) {
            bodyColor = enemy->currentColor();
        }

        const QBrush tankBrush = tankBrushForColor(bodyColor);
        if (item->brush() != tankBrush)
            item->setBrush(tankBrush);

        const QColor barrelColor = bodyColor.darker(190);
        if (directionItem->brush().color() != barrelColor)
            directionItem->setBrush(QBrush(barrelColor));

        const QPointF interpolatedPosition = tank->previousRenderPosition()
                                             + (tank->renderPosition() - tank->previousRenderPosition()) * alpha;
        const QPointF pos = tileToScene(interpolatedPosition);
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

void Renderer::syncBullets(const Game& game, qreal alpha)
{
    const Map* map = game.map();
    const qreal size = tileSize();
    QSet<const Bullet*> currentBullets;
    const qreal bulletLength = size * 0.9;
    const qreal bulletThickness = std::max<qreal>(size * 0.15, 2.0);

    for (Bullet* bullet : game.bullets()) {
        if (!bullet)
            continue;

        if (!bullet->isAlive()) {
            m_lastBulletCells.insert(bullet, bullet->cell());
            m_lastBulletExplosions.insert(bullet, bullet->spawnExplosionOnDestroy());
            continue;
        }

        currentBullets.insert(bullet);
        const bool vertical = bullet->direction() == Direction::Up || bullet->direction() == Direction::Down;
        const QSizeF bulletShape = vertical ? QSizeF(bulletThickness, bulletLength) : QSizeF(bulletLength, bulletThickness);
        const QPointF bulletOffset((size - bulletShape.width()) / 2.0, (size - bulletShape.height()) / 2.0);
        QGraphicsRectItem* item = m_bulletItems.value(bullet, nullptr);
        if (!item) {
            item = m_scene->addRect(QRectF(QPointF(0, 0), bulletShape), QPen(Qt::NoPen), QBrush(Qt::yellow));
            item->setZValue(20);
            m_bulletItems.insert(bullet, item);
        }

        if (item->rect().size() != bulletShape)
            item->setRect(QRectF(QPointF(0, 0), bulletShape));

        const QColor bulletColor = bullet->canPierceSteel() ? QColor(255, 180, 60) : QColor(255, 235, 80);
        if (item->brush().color() != bulletColor)
            item->setBrush(QBrush(bulletColor));

        const QPointF interpolatedPosition = bullet->previousRenderPosition()
                                             + (bullet->renderPosition() - bullet->previousRenderPosition()) * alpha;
        const QPointF pos = tileToScene(interpolatedPosition) + bulletOffset;
        item->setPos(pos);
        m_lastBulletCells.insert(bullet, bullet->cell());
        m_lastBulletExplosions.insert(bullet, bullet->spawnExplosionOnDestroy());
    }

    for (const Bullet* bullet : m_previousBullets) {
        if (!currentBullets.contains(bullet)) {
            const QPoint cell = m_lastBulletCells.value(bullet, QPoint(-1, -1));
            const bool shouldExplode = m_lastBulletExplosions.value(bullet, true);
            if (map && map->isInside(cell) && shouldExplode)
                m_explosions.append(Explosion{cell, kExplosionFrameCount, kExplosionFrameCount});
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
    const qreal hudX = std::min(m_renderOffset.x() + mapWidthInPixels + hudMargin,
                                m_scene->sceneRect().width() - hudMargin);
    const QPointF hudPosition(hudX, m_renderOffset.y() + hudMargin);

    if (!m_hudItem) {
        m_hudItem = new HudItem();
        m_scene->addItem(m_hudItem);
    }

    const int lives = game.state().remainingLives();
    const int enemyCount = game.state().aliveEnemies();
    const int score = game.state().score();
    const int stars = game.playerStars();
    const int maxStars = PlayerTank::maxStars();
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

    m_hudItem->setMetrics(lives, stars, maxStars, enemyCount, score, statusText, tileSize());

    if (m_hudItem->pos() != hudPosition)
        m_hudItem->setPos(hudPosition);
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

QPointF Renderer::cellToScene(const QPoint& cell) const
{
    return m_renderOffset + QPointF(cell) * tileSize();
}

QPointF Renderer::tileToScene(const QPointF& tile) const
{
    return m_renderOffset + tile * tileSize();
}

qreal Renderer::tileSize() const
{
    return m_tileScale;
}

void Renderer::updateExplosions()
{
    for (QGraphicsRectItem* item : m_explosionItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_explosionItems.clear();

    const qreal size = tileSize();

    for (Explosion& explosion : m_explosions) {
        if (explosion.ttlFrames <= 0)
            continue;

        const int frameIndex = explosion.totalFrames - explosion.ttlFrames;
        const int frameCount = std::max(1, explosion.totalFrames);
        const qreal frameProgress = frameCount > 1 ? static_cast<qreal>(frameIndex) / static_cast<qreal>(frameCount - 1) : 1.0;

        const qreal explosionScale = kExplosionMinScale + (kExplosionMaxScale - kExplosionMinScale) * frameProgress;
        const qreal explosionSize = size * explosionScale;
        const QPointF center = cellToScene(explosion.cell) + QPointF(size / 2.0, size / 2.0);
        const QPointF topLeft = center - QPointF(explosionSize / 2.0, explosionSize / 2.0);

        const QColor outerColor = QColor::fromRgbF(1.0, 0.8 - 0.25 * frameProgress, 0.25 + 0.2 * frameProgress);
        QGraphicsRectItem* core = m_scene->addRect(QRectF(topLeft, QSizeF(explosionSize, explosionSize)), QPen(Qt::NoPen), QBrush(outerColor));
        core->setZValue(25);
        m_explosionItems.append(core);

        const qreal flashThickness = explosionSize * 0.35;
        const QBrush flashBrush(QColor(255, 245, 180, 220));

        const QPointF horizontalPos(center.x() - explosionSize / 2.0, center.y() - flashThickness / 2.0);
        QGraphicsRectItem* horizontal = m_scene->addRect(QRectF(horizontalPos, QSizeF(explosionSize, flashThickness)), QPen(Qt::NoPen), flashBrush);
        horizontal->setZValue(26);
        m_explosionItems.append(horizontal);

        const QPointF verticalPos(center.x() - flashThickness / 2.0, center.y() - explosionSize / 2.0);
        QGraphicsRectItem* vertical = m_scene->addRect(QRectF(verticalPos, QSizeF(flashThickness, explosionSize)), QPen(Qt::NoPen), flashBrush);
        vertical->setZValue(26);
        m_explosionItems.append(vertical);

        --explosion.ttlFrames;
    }

    auto it = std::remove_if(m_explosions.begin(), m_explosions.end(), [](const Explosion& e) { return e.ttlFrames <= 0; });
    m_explosions.erase(it, m_explosions.end());
}
