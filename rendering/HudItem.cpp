#include "rendering/HudItem.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetricsF>
#include <QPen>
#include <QSizeF>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

namespace {
const QColor kHudLabelColor(210, 210, 210);
const QColor kHudLivesColor(230, 190, 60);
const QColor kHudEnemyColor(210, 70, 70);
const QColor kHudStatusColor(200, 200, 200);
const QColor kHudPanelColor(14, 14, 18, 230);
const QColor kHudPanelBorderColor(60, 60, 70, 230);
const QColor kHudShadowColor(0, 0, 0, 140);
const QColor kHudStarsColor(200, 200, 240);
const QColor kHudHighlightColor(255, 230, 140);
constexpr qreal kPi = 3.14159265358979323846;
} // namespace

HudItem::HudItem()
{
    setAcceptedMouseButtons(Qt::NoButton);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setZValue(100);
}

void HudItem::setMetrics(int lives, int stars, int maxStars, int enemies, int score, const QString& status, qreal tileSize)
{
    const bool dataChanged = m_lives != lives || m_stars != stars || m_maxStars != maxStars || m_enemies != enemies || m_score != score || m_statusText != status;
    const bool sizeChanged = !qFuzzyCompare(m_tileSize, tileSize);

    m_lives = lives;
    m_stars = stars;
    m_maxStars = maxStars;
    m_enemies = enemies;
    m_score = score;
    m_statusText = status;

    if (sizeChanged)
        updateFont(tileSize);

    if (dataChanged || sizeChanged)
        updateBounds();

    if (dataChanged || sizeChanged)
        update();
}

void HudItem::updateFont(qreal tileSize)
{
    m_tileSize = tileSize;
    QFont font(QStringLiteral("Monospace"));
    font.setStyleHint(QFont::TypeWriter, QFont::PreferBitmap);
    font.setWeight(QFont::DemiBold);
    const int pixelSize = std::clamp(static_cast<int>(std::round(tileSize * 0.75)), 12, 18);
    font.setPixelSize(pixelSize);
    m_font = font;

    QFontMetricsF metrics(m_font);
    m_labelWidth = metrics.horizontalAdvance(QStringLiteral("ENEMIES"));
    m_lineHeight = metrics.height();
    m_iconSize = std::clamp(tileSize * 0.95, 12.0, 22.0);
}

void HudItem::updateBounds()
{
    QFontMetricsF metrics(m_font);
    const qreal padding = 8.0;
    const qreal lineSpacing = 6.0;
    const qreal labelSpacing = 8.0;
    const qreal iconSpacing = 4.0;
    const int maxLifeIcons = 5;
    const int lifeIcons = std::clamp(m_lives, 0, maxLifeIcons);

    const qreal livesIconsWidth = lifeIcons > 0 ? lifeIcons * (m_iconSize + iconSpacing) - iconSpacing : 0.0;
    const qreal livesCountWidth = metrics.horizontalAdvance(QStringLiteral("x%1").arg(m_lives));
    const qreal livesValueWidth = livesIconsWidth + iconSpacing + livesCountWidth;

    const qreal starsIconsWidth = m_maxStars > 0 ? m_maxStars * (m_iconSize + iconSpacing) - iconSpacing : m_iconSize;
    const qreal enemyValueWidth = m_iconSize + iconSpacing + metrics.horizontalAdvance(QString::number(m_enemies));
    const QString scoreText = QStringLiteral("%1").arg(m_score, 7, 10, QLatin1Char('0'));
    const qreal scoreWidth = metrics.horizontalAdvance(scoreText);

    const qreal valueWidth = std::max({livesValueWidth, starsIconsWidth, enemyValueWidth, scoreWidth});
    const qreal panelWidth = padding * 2.0 + m_labelWidth + labelSpacing + valueWidth;

    qreal panelHeight = padding * 2.0;
    panelHeight += std::max(m_lineHeight, m_iconSize);                 // lives
    panelHeight += lineSpacing + std::max(m_lineHeight, m_iconSize);   // stars
    panelHeight += lineSpacing + std::max(m_lineHeight, m_iconSize);   // enemies
    panelHeight += lineSpacing + m_lineHeight;                         // score

    if (!m_statusText.isEmpty())
        panelHeight += lineSpacing + m_lineHeight;

    const QRectF newBounds(0.0, 0.0, panelWidth, panelHeight);
    if (newBounds != m_bounds) {
        prepareGeometryChange();
        m_bounds = newBounds;
    }
}

void HudItem::drawTankIcon(QPainter* painter, const QPointF& topLeft, qreal size) const
{
    const qreal bodyHeight = size * 0.65;
    const qreal barrelHeight = size - bodyHeight;
    const QRectF bodyRect(topLeft.x(), topLeft.y() + barrelHeight, size, bodyHeight);
    painter->fillRect(bodyRect, kHudLivesColor);

    const qreal trackWidth = size * 0.18;
    painter->fillRect(QRectF(bodyRect.left(), bodyRect.top(), trackWidth, bodyRect.height()), kHudShadowColor);
    painter->fillRect(QRectF(bodyRect.right() - trackWidth, bodyRect.top(), trackWidth, bodyRect.height()), kHudShadowColor);

    const qreal barrelWidth = size * 0.2;
    const QRectF barrelRect(topLeft.x() + (size - barrelWidth) / 2.0, topLeft.y(), barrelWidth, barrelHeight + size * 0.05);
    painter->fillRect(barrelRect, kHudHighlightColor);
}

void HudItem::drawStarIcon(QPainter* painter, const QPointF& center, qreal outerRadius, bool filled) const
{
    static constexpr int kPoints = 5;
    QPainterPath path;
    for (int i = 0; i < kPoints; ++i) {
        const qreal outerAngle = (72.0 * i - 90.0) * kPi / 180.0;
        const qreal innerAngle = outerAngle + 36.0 * kPi / 180.0;
        const QPointF outerPoint(center.x() + outerRadius * std::cos(outerAngle),
                                 center.y() + outerRadius * std::sin(outerAngle));
        const QPointF innerPoint(center.x() + outerRadius * 0.45 * std::cos(innerAngle),
                                 center.y() + outerRadius * 0.45 * std::sin(innerAngle));
        if (i == 0)
            path.moveTo(outerPoint);
        else
            path.lineTo(outerPoint);
        path.lineTo(innerPoint);
    }
    path.closeSubpath();

    painter->setBrush(filled ? kHudStarsColor : Qt::NoBrush);
    painter->setPen(QPen(kHudStarsColor, 1.2));
    painter->drawPath(path);
}

void HudItem::drawEnemyIcon(QPainter* painter, const QPointF& topLeft, qreal size) const
{
    const qreal trackHeight = size * 0.25;
    const QRectF baseRect(topLeft, QSizeF(size, size - trackHeight));
    painter->fillRect(baseRect, kHudEnemyColor);
    painter->fillRect(QRectF(baseRect.left(), baseRect.bottom() - trackHeight, baseRect.width(), trackHeight), kHudShadowColor);

    const qreal turretSize = size * 0.35;
    const QRectF turretRect(topLeft.x() + (size - turretSize) / 2.0,
                            topLeft.y() + (size - turretSize) / 2.0,
                            turretSize,
                            turretSize);
    painter->fillRect(turretRect, kHudLabelColor);
}

void HudItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF panelRect = m_bounds.adjusted(0.5, 0.5, -0.5, -0.5);
    painter->setBrush(kHudPanelColor);
    painter->setPen(QPen(kHudPanelBorderColor, 1.0));
    painter->drawRoundedRect(panelRect, 3.0, 3.0);

    QFontMetricsF metrics(m_font);
    painter->setFont(m_font);

    const qreal padding = 8.0;
    const qreal lineSpacing = 6.0;
    const qreal labelSpacing = 8.0;
    const qreal iconSpacing = 4.0;
    const qreal labelX = padding;
    const qreal valueX = padding + m_labelWidth + labelSpacing;

    qreal baseline = padding + metrics.ascent();

    painter->setPen(kHudLabelColor);
    painter->drawText(QPointF(labelX, baseline), QStringLiteral("LIVES"));

    const int maxLifeIcons = 5;
    const int lifeIcons = std::clamp(m_lives, 0, maxLifeIcons);
    const qreal livesTop = baseline - metrics.ascent() + (std::max(m_iconSize, m_lineHeight) - m_iconSize) / 2.0;
    qreal iconX = valueX;
    for (int i = 0; i < lifeIcons; ++i) {
        drawTankIcon(painter, QPointF(iconX, livesTop), m_iconSize);
        iconX += m_iconSize + iconSpacing;
    }

    painter->setPen(kHudLabelColor);
    painter->drawText(QPointF(iconX + iconSpacing, baseline), QStringLiteral("x%1").arg(m_lives));

    baseline += std::max(m_lineHeight, m_iconSize) + lineSpacing;
    painter->setPen(kHudLabelColor);
    painter->drawText(QPointF(labelX, baseline), QStringLiteral("STARS"));

    const qreal starsTop = baseline - metrics.ascent() + (std::max(m_iconSize, m_lineHeight) - m_iconSize) / 2.0;
    const qreal starRadius = m_iconSize * 0.5;
    iconX = valueX;
    const int totalStars = std::max(m_maxStars, 0);
    for (int i = 0; i < totalStars; ++i) {
        const bool filled = i < m_stars;
        drawStarIcon(painter, QPointF(iconX + starRadius, starsTop + starRadius), starRadius, filled);
        iconX += m_iconSize + iconSpacing;
    }

    baseline += std::max(m_lineHeight, m_iconSize) + lineSpacing;
    painter->setPen(kHudLabelColor);
    painter->drawText(QPointF(labelX, baseline), QStringLiteral("ENEMIES"));

    const qreal enemiesTop = baseline - metrics.ascent() + (std::max(m_iconSize, m_lineHeight) - m_iconSize) / 2.0;
    drawEnemyIcon(painter, QPointF(valueX, enemiesTop), m_iconSize);
    painter->setPen(kHudEnemyColor);
    painter->drawText(QPointF(valueX + m_iconSize + iconSpacing, baseline), QString::number(m_enemies));

    baseline += std::max(m_lineHeight, m_iconSize) + lineSpacing;
    painter->setPen(kHudLabelColor);
    painter->drawText(QPointF(labelX, baseline), QStringLiteral("SCORE"));

    const QString scoreText = QStringLiteral("%1").arg(m_score, 7, 10, QLatin1Char('0'));
    painter->setPen(kHudHighlightColor);
    painter->drawText(QPointF(valueX, baseline), scoreText);

    if (!m_statusText.isEmpty()) {
        baseline += m_lineHeight + lineSpacing;
        painter->setPen(kHudStatusColor);
        const QRectF statusRect(padding, baseline - metrics.ascent(), m_bounds.width() - padding * 2.0, m_lineHeight);
        painter->drawText(statusRect, Qt::AlignHCenter | Qt::AlignTop, m_statusText);
    }
}
