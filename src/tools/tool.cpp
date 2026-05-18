#include "tool.h"
#include "../ui/canvasview.h"
#include "../core/operation.h"
#include <QCursor>
#include <QtMath>

Tool::Tool(QObject *parent) : QObject(parent) {}

QCursor Tool::cursor() const
{
    return Qt::CrossCursor;
}

CropTool::CropTool(QObject *parent) : Tool(parent) {}

QCursor CropTool::cursor() const
{
    return Qt::CrossCursor;
}

void CropTool::onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        m_startPoint = pos;
        m_isDragging = true;
    }
}

void CropTool::onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons)
{
    if (m_isDragging) {
        QPointF topLeft(qMin(m_startPoint.x(), pos.x()), qMin(m_startPoint.y(), pos.y()));
        QSizeF size(qAbs(pos.x() - m_startPoint.x()), qAbs(pos.y() - m_startPoint.y()));

        if (!m_freeRatio && m_aspectRatio > 0) {
            if (size.width() / size.height() > m_aspectRatio) {
                size.setHeight(size.width() / m_aspectRatio);
            } else {
                size.setWidth(size.height() * m_aspectRatio);
            }
        }

        m_cropRect = QRect(topLeft.toPoint(), size.toSize());
        canvas->updateCropOverlay(m_cropRect);
    }
}

void CropTool::onMouseRelease(CanvasView *canvas, const QPointF &pos)
{
    if (m_isDragging) {
        QPointF topLeft(qMin(m_startPoint.x(), pos.x()), qMin(m_startPoint.y(), pos.y()));
        QSizeF size(qAbs(pos.x() - m_startPoint.x()), qAbs(pos.y() - m_startPoint.y()));

        if (!m_freeRatio && m_aspectRatio > 0) {
            if (size.width() / size.height() > m_aspectRatio) {
                size.setHeight(size.width() / m_aspectRatio);
            } else {
                size.setWidth(size.height() * m_aspectRatio);
            }
        }

        m_cropRect = QRect(topLeft.toPoint(), size.toSize());
        m_isDragging = false;
        canvas->updateCropOverlay(m_cropRect);
    }
}

Operation *CropTool::commitOperation(CanvasView *canvas)
{
    if (m_cropRect.isEmpty()) return nullptr;

    Operation *op = new CropOperation(m_cropRect);
    reset();
    return op;
}

void CropTool::reset()
{
    m_cropRect = QRect();
    m_isDragging = false;
}

BrushTool::BrushTool(QObject *parent) : Tool(parent) {}

QCursor BrushTool::cursor() const
{
    return Qt::CrossCursor;
}

void BrushTool::onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        m_strokePoints.clear();
        m_strokePoints.append({pos, 1.0});
        m_lastPoint = pos;
    }
}

void BrushTool::onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons)
{
    if (buttons & Qt::LeftButton) {
        qreal distance = QLineF(m_lastPoint, pos).length();
        qreal pressure = qBound(0.1, 1.0 - (distance / 50.0), 1.0);

        m_strokePoints.append({pos, pressure});
        m_lastPoint = pos;

        canvas->drawBrushPreview(pos, m_brushSize, m_color, m_opacity);
    }
}

void BrushTool::onMouseRelease(CanvasView *canvas, const QPointF &pos)
{
    Q_UNUSED(pos);
    canvas->clearBrushPreview();
}

Operation *BrushTool::commitOperation(CanvasView *canvas)
{
    Q_UNUSED(canvas);
    if (m_strokePoints.isEmpty()) return nullptr;

    Operation *op = new BrushOperation(m_strokePoints, m_brushSize, m_color, m_opacity);
    return op;
}

void BrushTool::reset()
{
    m_strokePoints.clear();
    m_lastPoint = QPointF();
}

BlurTool::BlurTool(QObject *parent) : Tool(parent) {}

QCursor BlurTool::cursor() const
{
    return Qt::CrossCursor;
}

void BlurTool::onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        m_points.clear();
        m_points.append(pos);
        m_startPoint = pos;
        m_isDragging = true;
    }
}

void BlurTool::onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons)
{
    if (m_isDragging && (buttons & Qt::LeftButton)) {
        m_points.append(pos);

        if (m_mode == BlurOperation::RectangleMode) {
            QPointF topLeft(qMin(m_startPoint.x(), pos.x()), qMin(m_startPoint.y(), pos.y()));
            QSizeF size(qAbs(pos.x() - m_startPoint.x()), qAbs(pos.y() - m_startPoint.y()));
            m_regionRect = QRect(topLeft.toPoint(), size.toSize());
            canvas->updateEffectOverlay(m_regionRect);
        }
    }
}

void BlurTool::onMouseRelease(CanvasView *canvas, const QPointF &pos)
{
    Q_UNUSED(pos);
    m_isDragging = false;
    canvas->clearEffectOverlay();
}

Operation *BlurTool::commitOperation(CanvasView *canvas)
{
    Q_UNUSED(canvas);
    if (m_points.isEmpty() && m_regionRect.isEmpty()) return nullptr;

    return new BlurOperation(m_points, m_radius, m_mode, m_regionRect);
}

void BlurTool::reset()
{
    m_points.clear();
    m_regionRect = QRect();
    m_isDragging = false;
}

PixelateTool::PixelateTool(QObject *parent) : Tool(parent) {}

QCursor PixelateTool::cursor() const
{
    return Qt::CrossCursor;
}

void PixelateTool::onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        m_points.clear();
        m_points.append(pos);
        m_startPoint = pos;
        m_isDragging = true;
    }
}

void PixelateTool::onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons)
{
    if (m_isDragging && (buttons & Qt::LeftButton)) {
        m_points.append(pos);

        if (m_mode == PixelateOperation::RectangleMode) {
            QPointF topLeft(qMin(m_startPoint.x(), pos.x()), qMin(m_startPoint.y(), pos.y()));
            QSizeF size(qAbs(pos.x() - m_startPoint.x()), qAbs(pos.y() - m_startPoint.y()));
            m_regionRect = QRect(topLeft.toPoint(), size.toSize());
            canvas->updateEffectOverlay(m_regionRect);
        }
    }
}

void PixelateTool::onMouseRelease(CanvasView *canvas, const QPointF &pos)
{
    Q_UNUSED(pos);
    m_isDragging = false;
    canvas->clearEffectOverlay();
}

Operation *PixelateTool::commitOperation(CanvasView *canvas)
{
    Q_UNUSED(canvas);
    if (m_points.isEmpty() && m_regionRect.isEmpty()) return nullptr;

    return new PixelateOperation(m_points, m_pixelSize, m_mode, m_regionRect);
}

void PixelateTool::reset()
{
    m_points.clear();
    m_regionRect = QRect();
    m_isDragging = false;
}

TextTool::TextTool(QObject *parent) : Tool(parent) {}

QCursor TextTool::cursor() const
{
    return Qt::IBeamCursor;
}

void TextTool::onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        m_position = pos;
    }
}

void TextTool::onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons)
{
    Q_UNUSED(canvas);
    Q_UNUSED(pos);
    Q_UNUSED(buttons);
}

void TextTool::onMouseRelease(CanvasView *canvas, const QPointF &pos)
{
    m_position = pos;
    canvas->showTextEditor(m_position, m_text, m_fontFamily, m_fontSize, m_color, m_opacity, m_rotation);
}

Operation *TextTool::commitOperation(CanvasView *canvas)
{
    Q_UNUSED(canvas);
    if (m_text.isEmpty()) return nullptr;

    return new TextOperation(m_text, m_position, m_fontFamily, m_fontSize, m_color, m_opacity, m_rotation);
}

void TextTool::reset()
{
    m_text = "Text";
}

EraserTool::EraserTool(QObject *parent) : Tool(parent)
{
    m_color = Qt::white;
}

QCursor EraserTool::cursor() const
{
    return Qt::CrossCursor;
}

void EraserTool::onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        m_strokePoints.clear();
        m_strokePoints.append({pos, 1.0});
        m_lastPoint = pos;
    }
}

void EraserTool::onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons)
{
    if (buttons & Qt::LeftButton) {
        qreal distance = QLineF(m_lastPoint, pos).length();
        qreal pressure = qBound(0.1, 1.0 - (distance / 50.0), 1.0);

        m_strokePoints.append({pos, pressure});
        m_lastPoint = pos;
    }
}

void EraserTool::onMouseRelease(CanvasView *canvas, const QPointF &pos)
{
    Q_UNUSED(pos);
}

Operation *EraserTool::commitOperation(CanvasView *canvas)
{
    Q_UNUSED(canvas);
    if (m_strokePoints.isEmpty()) return nullptr;

    return new BrushOperation(m_strokePoints, m_brushSize, Qt::transparent, 1.0);
}

void EraserTool::reset()
{
    m_strokePoints.clear();
    m_lastPoint = QPointF();
}