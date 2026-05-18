#ifndef BRUSH_H
#define BRUSH_H

#include <QVector>
#include <QPointF>
#include <QColor>

struct BrushStrokePoint
{
    QPointF position;
    qreal pressure;
};

class Brush
{
public:
    Brush();
    ~Brush();

    void beginStroke(const QPointF &pos);
    void addPoint(const QPointF &pos, qreal pressure = 1.0);
    void endStroke();

    QVector<BrushStrokePoint> points() const { return m_points; }
    void clear();

    void setSize(int size) { m_size = size; }
    int size() const { return m_size; }

    void setColor(const QColor &color) { m_color = color; }
    QColor color() const { return m_color; }

    void setOpacity(qreal opacity) { m_opacity = opacity; }
    qreal opacity() const { return m_opacity; }

    bool hasStroke() const { return !m_points.isEmpty(); }

private:
    QVector<BrushStrokePoint> m_points;
    int m_size = 10;
    QColor m_color = Qt::red;
    qreal m_opacity = 1.0;
};

#endif