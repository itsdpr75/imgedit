#ifndef CANVASITEM_H
#define CANVASITEM_H

#include <QGraphicsItem>

class CanvasItem : public QGraphicsItem
{
public:
    explicit CanvasItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setImage(const QImage &image);
    QImage image() const { return m_image; }

private:
    QImage m_image;
};

#endif