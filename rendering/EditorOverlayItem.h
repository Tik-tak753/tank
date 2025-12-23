#ifndef EDITOROVERLAYITEM_H
#define EDITOROVERLAYITEM_H

#include <QFont>
#include <QGraphicsItem>
#include <QRectF>
#include <QVector>
#include <QString>
#include <QColor>
#include <functional>

#include "world/Tile.h"

class QGraphicsSceneMouseEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class EditorOverlayItem : public QGraphicsItem
{
public:
    EditorOverlayItem();

    void setTileSize(qreal size);
    void setSelectedTile(TileType type);
    void setSelectionCallback(std::function<void(TileType)> callback);

    QRectF boundingRect() const override { return m_bounds; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    struct TileButton {
        TileType type;
        QString label;
        QColor color;
        QColor accent;
        QRectF rect;
    };

    void rebuildLayout();
    int buttonIndexAt(const QPointF& pos) const;
    void drawTileIcon(QPainter* painter, const TileButton& button) const;
    void drawHelpText(QPainter* painter, qreal top) const;

    QVector<TileButton> m_buttons;
    QVector<QString> m_helpLines;
    QRectF m_bounds{0.0, 0.0, 1.0, 1.0};
    QFont m_font;
    qreal m_tileSize = 28.0;
    qreal m_margin = 10.0;
    qreal m_spacing = 8.0;
    qreal m_labelGap = 6.0;
    TileType m_selectedType = TileType::Brick;
    std::function<void(TileType)> m_selectionCallback;
};

#endif // EDITOROVERLAYITEM_H
