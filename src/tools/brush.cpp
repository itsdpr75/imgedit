#include "brush.h"
#include <QLineF>
#include <QDebug>

Brush::Brush() {}

Brush::~Brush() {}

void Brush::beginStroke(const QPointF &pos)
{
    m_points.clear();
    m_points.append({pos, 1.0});
}

void Brush::addPoint(const QPointF &pos, qreal pressure)
{
    if (!m_points.isEmpty()) {
        qreal distance = QLineF(m_points.last().position, pos).length();
        if (distance < 2) return;
    }
    m_points.append({pos, pressure});
}

void Brush::endStroke()
{
}

void Brush::clear()
{
    m_points.clear();
}