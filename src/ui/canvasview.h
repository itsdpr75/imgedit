#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <QGraphicsView>
#include <QPointF>
#include <QRect>
#include <QColor>
#include <QImage>
#include <QGraphicsTextItem>

#include "../core/operation.h"

class Tool;
class ImageDocument;
class TextItem;

class CanvasView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CanvasView(QWidget *parent = nullptr);

    void setDocument(ImageDocument *doc);
    ImageDocument *document() const { return m_document; }

    void setTool(Tool *tool);
    Tool *currentTool() const { return m_tool; }

    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoom100();
    void setZoom(qreal zoom);

    qreal zoom() const { return m_zoom; }

    void drawBrushPreview(const QPointF &pos, int size, const QColor &color, qreal opacity);
    void clearBrushPreview();
    void updateCropOverlay(const QRect &rect);
    void clearCropOverlay();
    void updateEffectOverlay(const QRect &rect);
    void clearEffectOverlay();

    void showTextEditor(const QPointF &pos, const QString &text,
                        const QString &fontFamily, int fontSize,
                        const QColor &color, qreal opacity, int rotation);

signals:
    void toolApplied(Operation *operation);
    void textConfirmed(const QString &text, const QPointF &pos,
                      const QString &fontFamily, int fontSize,
                      const QColor &color, qreal opacity, int rotation);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onImageChanged(const QImage &image);

private:
    void updateSceneFromImage();
    QPointF mapToImage(const QPointF &pos) const;

    ImageDocument *m_document = nullptr;
    Tool *m_tool = nullptr;

    qreal m_zoom = 1.0;
    QPointF m_panOffset;

    QGraphicsPixmapItem *m_imageItem = nullptr;
    QGraphicsItem *m_brushPreviewItem = nullptr;
    QGraphicsItem *m_cropOverlayItem = nullptr;
    QGraphicsItem *m_effectOverlayItem = nullptr;
    TextItem *m_textItem = nullptr;

    QPointF m_lastPanPos;
    bool m_isPanning = false;
};

class TextItem : public QGraphicsTextItem
{
public:
    explicit TextItem(QGraphicsItem *parent = nullptr);

    void setEditMode(bool edit) { m_editMode = edit; }
    bool isEditMode() const { return m_editMode; }

    void setTextRotation(int rotation) { m_rotation = rotation; }
    int textRotation() const { return m_rotation; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    bool m_editMode = true;
    int m_rotation = 0;
};

#endif