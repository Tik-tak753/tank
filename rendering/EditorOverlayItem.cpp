#include "rendering/EditorOverlayItem.h"

#include <QColor>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QtGlobal>

namespace {
QColor lighten(const QColor& base, int amount)
{
    QColor color = base;
    color = color.lighter(amount);
    color.setAlpha(255);
    return color;
}
} // namespace

EditorOverlayItem::EditorOverlayItem()
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setZValue(1000.0);

    m_font.setFamily(QStringLiteral("Montserrat"));
    m_font.setPixelSize(12);

    m_buttons = {
        {TileType::Empty,  QStringLiteral("Empty"),  QColor(35, 35, 35), QColor(90, 90, 90), QRectF()},
        {TileType::Brick,  QStringLiteral("Brick"),  QColor(193, 68, 14), QColor(255, 200, 160), QRectF()},
        {TileType::Steel,  QStringLiteral("Steel"),  QColor(170, 170, 180), QColor(230, 230, 235), QRectF()},
        {TileType::Forest, QStringLiteral("Forest"), QColor(60, 140, 80, 220), QColor(120, 200, 130, 240), QRectF()},
        {TileType::Water,  QStringLiteral("Water"),  QColor(50, 120, 210), QColor(110, 180, 255), QRectF()},
        {TileType::Ice,    QStringLiteral("Ice"),    QColor(210, 230, 240), QColor(255, 255, 255), QRectF()},
    };

    m_helpLines = {
        QStringLiteral("1-6: pick tile, 7: keep current"),
        QStringLiteral("Left click: place  |  Right click: erase"),
        QStringLiteral("Ctrl+S: save  |  Ctrl+O: load"),
    };

    rebuildLayout();
}

void EditorOverlayItem::setTileSize(qreal size)
{
    const qreal clamped = qBound<qreal>(18.0, size, 56.0);
    if (qFuzzyCompare(m_tileSize, clamped))
        return;

    m_tileSize = clamped;
    rebuildLayout();
    update();
}

void EditorOverlayItem::setSelectedTile(TileType type)
{
    if (m_selectedType == type)
        return;

    m_selectedType = type;
    update();
}

void EditorOverlayItem::setSelectionCallback(std::function<void(TileType)> callback)
{
    m_selectionCallback = std::move(callback);
}

void EditorOverlayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setFont(m_font);

    // background
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(12, 12, 16, 200));
    painter->drawRoundedRect(m_bounds, 8.0, 8.0);

    const qreal highlightPadding = 4.0;
    const QColor borderColor(255, 255, 255, 50);
    painter->setPen(QPen(borderColor, 1.0));
    painter->setBrush(Qt::NoBrush);

    for (const TileButton& button : m_buttons) {
        const QRectF rect = button.rect;
        if (button.type == m_selectedType) {
            const QRectF highlightRect = rect.adjusted(-highlightPadding, -highlightPadding,
                                                       highlightPadding, highlightPadding);
            painter->setBrush(QColor(255, 215, 80, 80));
            painter->drawRoundedRect(highlightRect, 6.0, 6.0);
        }

        painter->setPen(QPen(borderColor, 1.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect, 4.0, 4.0);

        drawTileIcon(painter, button);

        const QRectF labelRect = QRectF(rect.topLeft() + QPointF(m_tileSize + m_labelGap, 0.0),
                                        QSizeF(rect.width() - m_tileSize - m_labelGap, rect.height()));
        painter->setPen(QColor(235, 235, 240));
        painter->drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft, button.label);
    }

    const qreal helpTop = m_bounds.height() - m_margin - (m_helpLines.size() * (m_font.pixelSize() + 2));
    drawHelpText(painter, helpTop);
}

void EditorOverlayItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    const int index = buttonIndexAt(event->pos());
    if (index < 0 || index >= m_buttons.size()) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    const TileType type = m_buttons.at(index).type;
    setSelectedTile(type);
    if (m_selectionCallback)
        m_selectionCallback(type);

    event->accept();
}

void EditorOverlayItem::rebuildLayout()
{
    const QFontMetricsF metrics(m_font);
    qreal maxLabelWidth = 0.0;
    qreal rowHeight = qMax(m_tileSize, metrics.height());
    for (const TileButton& button : m_buttons)
        maxLabelWidth = qMax(maxLabelWidth, metrics.horizontalAdvance(button.label));

    const int columns = 3;
    const int rows = (m_buttons.size() + columns - 1) / columns;

    const qreal tileWidth = m_tileSize + m_labelGap + maxLabelWidth;
    const qreal rowSpacing = m_spacing;
    const qreal colSpacing = m_spacing;

    qreal y = m_margin;
    int index = 0;
    for (int row = 0; row < rows; ++row) {
        qreal x = m_margin;
        for (int col = 0; col < columns && index < m_buttons.size(); ++col, ++index) {
            TileButton& button = m_buttons[index];
            button.rect = QRectF(x, y, tileWidth, rowHeight);
            x += tileWidth + colSpacing;
        }
        y += rowHeight + rowSpacing;
    }

    const qreal helpHeight = m_helpLines.size() * (metrics.height() + 2.0);
    const qreal width = (tileWidth * columns) + colSpacing * (columns - 1) + 2 * m_margin;
    const qreal height = y - rowSpacing + helpHeight + m_margin * 2;

    prepareGeometryChange();
    m_bounds = QRectF(0.0, 0.0, width, height);
}

int EditorOverlayItem::buttonIndexAt(const QPointF& pos) const
{
    for (int i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons.at(i).rect.contains(pos))
            return i;
    }
    return -1;
}

void EditorOverlayItem::drawTileIcon(QPainter* painter, const TileButton& button) const
{
    const QRectF iconRect(button.rect.topLeft(), QSizeF(m_tileSize, m_tileSize));
    painter->setPen(Qt::NoPen);

    if (button.type == TileType::Empty) {
        painter->setBrush(QColor(30, 30, 30));
        painter->drawRect(iconRect);
        painter->setPen(QPen(QColor(140, 140, 140), 2.0));
        painter->drawLine(iconRect.topLeft() + QPointF(4.0, 4.0), iconRect.bottomRight() - QPointF(4.0, 4.0));
        painter->drawLine(iconRect.bottomLeft() + QPointF(4.0, -4.0), iconRect.topRight() - QPointF(4.0, -4.0));
        return;
    }

    if (button.type == TileType::Forest) {
        painter->setBrush(button.color);
        painter->drawRect(iconRect);
        painter->setBrush(button.accent);
        const qreal patch = qMax<qreal>(2.0, m_tileSize / 5.0);
        for (qreal y = iconRect.top(); y < iconRect.bottom(); y += patch) {
            const qreal offset = static_cast<int>((y - iconRect.top()) / patch) % 2 ? patch / 2.0 : 0.0;
            for (qreal x = iconRect.left() + offset; x < iconRect.right(); x += patch)
                painter->drawRect(QRectF(QPointF(x, y), QSizeF(patch, patch)));
        }
        return;
    }

    if (button.type == TileType::Water) {
        QLinearGradient gradient(iconRect.topLeft(), iconRect.bottomLeft());
        gradient.setColorAt(0.0, lighten(button.color, 110));
        gradient.setColorAt(1.0, button.color);
        painter->setBrush(gradient);
        painter->drawRect(iconRect);
        painter->setBrush(button.accent);
        const qreal stripe = qMax<qreal>(2.0, m_tileSize / 6.0);
        for (qreal y = iconRect.top() + stripe / 2.0; y < iconRect.bottom(); y += stripe * 2.0)
            painter->drawRect(QRectF(QPointF(iconRect.left(), y), QSizeF(iconRect.width(), stripe)));
        return;
    }

    if (button.type == TileType::Ice) {
        painter->setBrush(button.color);
        painter->drawRect(iconRect);
        painter->setBrush(QColor(255, 255, 255, 120));
        painter->drawRect(iconRect.adjusted(2.0, 2.0, -2.0, -2.0));
        painter->setBrush(QColor(255, 255, 255, 40));
        painter->drawRect(iconRect.adjusted(4.0, 4.0, -6.0, -6.0));
        return;
    }

    if (button.type == TileType::Steel) {
        painter->setBrush(button.color);
        painter->drawRect(iconRect);
        painter->setBrush(QColor(90, 90, 100));
        const qreal border = qMax<qreal>(2.0, m_tileSize / 9.0);
        painter->drawRect(iconRect.adjusted(border, border, -border, -border));
        painter->setBrush(button.accent);
        const qreal groove = qMax<qreal>(1.5, m_tileSize / 14.0);
        painter->drawRect(QRectF(iconRect.center().x() - groove / 2.0, iconRect.top() + border,
                                 groove, iconRect.height() - 2 * border));
        painter->drawRect(QRectF(iconRect.left() + border, iconRect.center().y() - groove / 2.0,
                                 iconRect.width() - 2 * border, groove));
        return;
    }

    painter->setBrush(button.color);
    painter->drawRect(iconRect);
    painter->setBrush(button.accent);
    painter->drawRect(QRectF(iconRect.left(), iconRect.top(), iconRect.width(), qMax<qreal>(2.0, m_tileSize / 8.0)));
    painter->drawRect(QRectF(iconRect.left(), iconRect.bottom() - qMax<qreal>(2.0, m_tileSize / 9.0),
                             iconRect.width(), qMax<qreal>(2.0, m_tileSize / 9.0)));
}

void EditorOverlayItem::drawHelpText(QPainter* painter, qreal top) const
{
    painter->setPen(QColor(210, 210, 215));
    qreal y = top;
    for (const QString& line : m_helpLines) {
        painter->drawText(QPointF(m_margin, y + m_font.pixelSize()), line);
        y += m_font.pixelSize() + 2.0;
    }
}
