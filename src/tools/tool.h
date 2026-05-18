#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include <QPointF>
#include <QColor>
#include <QCursor>
#include <QImage>
#include <QString>
#include <QVector>

#include "../core/operation.h"

class CanvasView;

enum ToolType {
    ToolSelect,
    ToolCrop,
    ToolBrush,
    ToolEraser,
    ToolBlur,
    ToolPixelate,
    ToolText
};

class Tool : public QObject
{
    Q_OBJECT

public:
    explicit Tool(QObject *parent = nullptr);
    virtual ~Tool() = default;

    virtual ToolType type() const = 0;
    virtual QString name() const = 0;
    virtual QCursor cursor() const;

    virtual void onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button) = 0;
    virtual void onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons) = 0;
    virtual void onMouseRelease(CanvasView *canvas, const QPointF &pos) = 0;
    virtual void onWheel(CanvasView *canvas, const QPointF &pos, int delta, Qt::KeyboardModifiers modifiers) {}

    virtual Operation *commitOperation(CanvasView *canvas) = 0;
    virtual void reset() = 0;

    void setColor(const QColor &color) { m_color = color; }
    QColor color() const { return m_color; }

    void setBrushSize(int size) { m_brushSize = size; }
    int brushSize() const { return m_brushSize; }

    void setOpacity(qreal opacity) { m_opacity = opacity; }
    qreal opacity() const { return m_opacity; }

signals:
    void toolChanged();

protected:
    QColor m_color = Qt::red;
    int m_brushSize = 10;
    qreal m_opacity = 1.0;
    QVector<QPointF> m_currentStroke;
};

class CropTool : public Tool
{
public:
    explicit CropTool(QObject *parent = nullptr);

    ToolType type() const override { return ToolCrop; }
    QString name() const override { return "Crop"; }
    QCursor cursor() const override;

    void onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button) override;
    void onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons) override;
    void onMouseRelease(CanvasView *canvas, const QPointF &pos) override;

    Operation *commitOperation(CanvasView *canvas) override;
    void reset() override;

    void setAspectRatio(qreal ratio) { m_aspectRatio = ratio; }
    qreal aspectRatio() const { return m_aspectRatio; }

    void setFreeRatio(bool free) { m_freeRatio = free; }
    bool isFreeRatio() const { return m_freeRatio; }

    QRect cropRect() const { return m_cropRect; }

private:
    QPointF m_startPoint;
    QRect m_cropRect;
    bool m_isDragging = false;
    qreal m_aspectRatio = 0;
    bool m_freeRatio = true;
};

class BrushTool : public Tool
{
public:
    explicit BrushTool(QObject *parent = nullptr);

    ToolType type() const override { return ToolBrush; }
    QString name() const override { return "Brush"; }
    QCursor cursor() const override;

    void onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button) override;
    void onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons) override;
    void onMouseRelease(CanvasView *canvas, const QPointF &pos) override;

    Operation *commitOperation(CanvasView *canvas) override;
    void reset() override;

private:
    QVector<BrushOperation::StrokePoint> m_strokePoints;
    QPointF m_lastPoint;
};

class BlurTool : public Tool
{
public:
    explicit BlurTool(QObject *parent = nullptr);

    ToolType type() const override { return ToolBlur; }
    QString name() const override { return "Blur"; }
    QCursor cursor() const override;

    void setRadius(int radius) { m_radius = radius; }
    int radius() const { return m_radius; }

    void setMode(BlurOperation::Mode mode) { m_mode = mode; }
    BlurOperation::Mode mode() const { return m_mode; }

    void onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button) override;
    void onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons) override;
    void onMouseRelease(CanvasView *canvas, const QPointF &pos) override;

    Operation *commitOperation(CanvasView *canvas) override;
    void reset() override;

private:
    int m_radius = 20;
    BlurOperation::Mode m_mode = BlurOperation::BrushMode;
    QVector<QPointF> m_points;
    QPointF m_startPoint;
    QRect m_regionRect;
    bool m_isDragging = false;
};

class PixelateTool : public Tool
{
public:
    explicit PixelateTool(QObject *parent = nullptr);

    ToolType type() const override { return ToolPixelate; }
    QString name() const override { return "Pixelate"; }
    QCursor cursor() const override;

    void setPixelSize(int size) { m_pixelSize = size; }
    int pixelSize() const { return m_pixelSize; }

    void setMode(PixelateOperation::Mode mode) { m_mode = mode; }
    PixelateOperation::Mode mode() const { return m_mode; }

    void onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button) override;
    void onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons) override;
    void onMouseRelease(CanvasView *canvas, const QPointF &pos) override;

    Operation *commitOperation(CanvasView *canvas) override;
    void reset() override;

private:
    int m_pixelSize = 10;
    PixelateOperation::Mode m_mode = PixelateOperation::BrushMode;
    QVector<QPointF> m_points;
    QPointF m_startPoint;
    QRect m_regionRect;
    bool m_isDragging = false;
};

class TextTool : public Tool
{
public:
    explicit TextTool(QObject *parent = nullptr);

    ToolType type() const override { return ToolText; }
    QString name() const override { return "Text"; }
    QCursor cursor() const override;

    void setText(const QString &text) { m_text = text; }
    QString text() const { return m_text; }

    void setFontFamily(const QString &family) { m_fontFamily = family; }
    QString fontFamily() const { return m_fontFamily; }

    void setFontSize(int size) { m_fontSize = size; }
    int fontSize() const { return m_fontSize; }

    void setRotation(int rotation) { m_rotation = rotation; }
    int rotation() const { return m_rotation; }

    void onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button) override;
    void onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons) override;
    void onMouseRelease(CanvasView *canvas, const QPointF &pos) override;

    Operation *commitOperation(CanvasView *canvas) override;
    void reset() override;

private:
    QString m_text = "Text";
    QString m_fontFamily = "Sans";
    int m_fontSize = 24;
    int m_rotation = 0;
    QPointF m_position;
};

class EraserTool : public Tool
{
public:
    explicit EraserTool(QObject *parent = nullptr);

    ToolType type() const override { return ToolEraser; }
    QString name() const override { return "Eraser"; }
    QCursor cursor() const override;

    void onMousePress(CanvasView *canvas, const QPointF &pos, Qt::MouseButton button) override;
    void onMouseMove(CanvasView *canvas, const QPointF &pos, Qt::MouseButtons buttons) override;
    void onMouseRelease(CanvasView *canvas, const QPointF &pos) override;

    Operation *commitOperation(CanvasView *canvas) override;
    void reset() override;

private:
    QVector<BrushOperation::StrokePoint> m_strokePoints;
    QPointF m_lastPoint;
};

#endif