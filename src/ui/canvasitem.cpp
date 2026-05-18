#include "canvasitem.h"
#include <QPainter>

CanvasItem::CanvasItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
}

QRectF CanvasItem::boundingRect() const
{
    return m_image.rect();
}

void CanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (!m_image.isNull()) {
        painter->drawImage(0, 0, m_image);
    }
}