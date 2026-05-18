#include "canvasview.h"
#include "../core/imagedocument.h"
#include "../core/operation.h"
#include "../tools/tool.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QMimeData>
#include <QFile>
#include <QImage>
#include <QPen>
#include <QBrush>
#include <QScrollBar>

class PreviewItem : public QGraphicsRectItem
{
public:
    PreviewItem() : QGraphicsRectItem() {}
    void setPreviewPen(const QPen &pen) { m_previewPen = pen; update(); }
    void setPreviewBrush(const QBrush &brush) { m_previewBrush = brush; update(); }
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
    {
        painter->setPen(m_previewPen);
        painter->setBrush(m_previewBrush);
        painter->drawRect(rect());
    }
private:
    QPen m_previewPen;
    QBrush m_previewBrush;
};

CanvasView::CanvasView(QWidget *parent)
    : QGraphicsView(parent)
{
    setScene(new QGraphicsScene(this));
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setBackgroundBrush(QColor(35, 35, 35));
    setFrameShape(NoFrame);

    m_imageItem = scene()->addPixmap(QPixmap());
    m_imageItem->setZValue(0);

    m_brushPreviewItem = new PreviewItem();
    m_brushPreviewItem->setZValue(10);
    m_brushPreviewItem->setVisible(false);
    scene()->addItem(m_brushPreviewItem);

    m_cropOverlayItem = new PreviewItem();
    m_cropOverlayItem->setZValue(20);
    m_cropOverlayItem->setVisible(false);
    scene()->addItem(m_cropOverlayItem);

    m_effectOverlayItem = new PreviewItem();
    m_effectOverlayItem->setZValue(20);
    m_effectOverlayItem->setVisible(false);
    scene()->addItem(m_effectOverlayItem);
}

void CanvasView::setDocument(ImageDocument *doc)
{
    if (m_document) {
        disconnect(m_document, &ImageDocument::imageChanged, this, &CanvasView::onImageChanged);
    }

    m_document = doc;

    if (m_document) {
        connect(m_document, &ImageDocument::imageChanged, this, &CanvasView::onImageChanged);
        updateSceneFromImage();
    }
}

void CanvasView::setTool(Tool *tool)
{
    m_tool = tool;
    if (m_tool) {
        setCursor(m_tool->cursor());
    }
}

void CanvasView::zoomIn()
{
    setZoom(m_zoom * 1.25);
}

void CanvasView::zoomOut()
{
    setZoom(m_zoom / 1.25);
}

void CanvasView::zoomFit()
{
    if (!m_document || m_document->originalImage().isNull()) return;

    QRectF imageRect = m_document->originalImage().rect();
    fitInView(imageRect, Qt::KeepAspectRatio);
    m_zoom = transform().m11();
}

void CanvasView::zoom100()
{
    resetTransform();
    m_zoom = 1.0;
}

void CanvasView::setZoom(qreal zoom)
{
    m_zoom = qBound(0.01, zoom, 50.0);
    QTransform transform;
    transform.scale(m_zoom, m_zoom);
    setTransform(transform);
}

void CanvasView::drawBrushPreview(const QPointF &pos, int size, const QColor &color, qreal opacity)
{
    Q_UNUSED(opacity);
    PreviewItem *item = static_cast<PreviewItem*>(m_brushPreviewItem);
    item->setRect(pos.x() - size / 2.0, pos.y() - size / 2.0, size, size);
    item->setPreviewPen(QPen(color, 2));
    item->setPreviewBrush(Qt::NoBrush);
    item->setVisible(true);
    scene()->update();
}

void CanvasView::clearBrushPreview()
{
    m_brushPreviewItem->setVisible(false);
    scene()->update();
}

void CanvasView::updateCropOverlay(const QRect &rect)
{
    PreviewItem *item = static_cast<PreviewItem*>(m_cropOverlayItem);
    item->setRect(rect);
    item->setPreviewPen(QPen(QColor(42, 130, 218), 2));
    item->setPreviewBrush(QBrush(QColor(42, 130, 218, 50)));
    item->setVisible(true);
    scene()->update();
}

void CanvasView::clearCropOverlay()
{
    m_cropOverlayItem->setVisible(false);
    scene()->update();
}

void CanvasView::updateEffectOverlay(const QRect &rect)
{
    PreviewItem *item = static_cast<PreviewItem*>(m_effectOverlayItem);
    item->setRect(rect);
    item->setPreviewPen(QPen(QColor(255, 200, 0, 200), 1));
    item->setPreviewBrush(QBrush(QColor(255, 200, 0, 30)));
    item->setVisible(true);
    scene()->update();
}

void CanvasView::clearEffectOverlay()
{
    m_effectOverlayItem->setVisible(false);
    scene()->update();
}

void CanvasView::showTextEditor(const QPointF &pos, const QString &text,
                                const QString &fontFamily, int fontSize,
                                const QColor &color, qreal opacity, int rotation)
{
    Q_UNUSED(pos);
    Q_UNUSED(text);
    Q_UNUSED(fontFamily);
    Q_UNUSED(fontSize);
    Q_UNUSED(color);
    Q_UNUSED(opacity);
    Q_UNUSED(rotation);

    emit textConfirmed(text, pos, fontFamily, fontSize, color, opacity, rotation);
}

void CanvasView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier)) {
        m_isPanning = true;
        m_lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (m_tool && event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        QPointF imagePos = mapToImage(scenePos);
        m_tool->onMousePress(this, imagePos, event->button());
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPointF delta = event->pos() - m_lastPanPos;
        m_lastPanPos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }

    if (m_tool) {
        QPointF scenePos = mapToScene(event->pos());
        QPointF imagePos = mapToImage(scenePos);
        m_tool->onMouseMove(this, imagePos, event->buttons());
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && m_isPanning)) {
        m_isPanning = false;
        setCursor(m_tool ? m_tool->cursor() : Qt::ArrowCursor);
        event->accept();
        return;
    }

    if (m_tool && event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        QPointF imagePos = mapToImage(scenePos);
        m_tool->onMouseRelease(this, imagePos);

        Operation *op = m_tool->commitOperation(this);
        if (op) {
            emit toolApplied(op);
        }
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void CanvasView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CanvasView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().first();
        QString path = url.toLocalFile();
        if (!path.isEmpty()) {
            emit this->window()->windowTitleChanged(QString());
        }
    }
}

void CanvasView::onImageChanged(const QImage &image)
{
    qDebug() << "[CanvasView] onImageChanged received, isNull:" << image.isNull();
    updateSceneFromImage();
}

void CanvasView::updateSceneFromImage()
{
    qDebug() << "[CanvasView] updateSceneFromImage called";
    if (m_document && !m_document->originalImage().isNull()) {
        QImage img = m_document->currentImage();
        qDebug() << "[CanvasView] displaying currentImage, isNull:" << img.isNull() << "size:" << img.size();
        if (img.isNull()) {
            img = m_document->originalImage();
        }
        m_imageItem->setPixmap(QPixmap::fromImage(img));
        scene()->setSceneRect(img.rect());

        m_brushPreviewItem->setVisible(false);
        m_cropOverlayItem->setVisible(false);
        m_effectOverlayItem->setVisible(false);

        if (m_zoom == 1.0) {
            zoomFit();
        }
    }
}

QPointF CanvasView::mapToImage(const QPointF &pos) const
{
    QPointF scenePos = mapToScene(pos.toPoint());
    qreal imageX = scenePos.x();
    qreal imageY = scenePos.y();
    return QPointF(imageX, imageY);
}

TextItem::TextItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_rotation != 0) {
        painter->save();
        QPointF center = boundingRect().center();
        painter->translate(center);
        painter->rotate(m_rotation);
        painter->translate(-center);
    }

    QGraphicsTextItem::paint(painter, nullptr, nullptr);

    if (m_rotation != 0) {
        painter->restore();
    }
}