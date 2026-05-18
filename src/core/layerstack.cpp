#include "layerstack.h"
#include <QPainter>

Layer::Layer(const QString &name) : m_name(name) {}
Layer::~Layer() {}

LayerStack::LayerStack(QObject *parent) : QObject(parent) {}
LayerStack::~LayerStack() {}

void LayerStack::addLayer(Layer *layer)
{
    m_layers.append(layer);
    emit layerChanged();
}

void LayerStack::removeLayer(int index)
{
    if (index >= 0 && index < m_layers.size()) {
        delete m_layers.takeAt(index);
        emit layerChanged();
    }
}

void LayerStack::moveLayer(int from, int to)
{
    if (from >= 0 && from < m_layers.size() && to >= 0 && to < m_layers.size()) {
        m_layers.move(from, to);
        emit layerChanged();
    }
}

void LayerStack::mergeDown(int index)
{
    if (index > 0 && index < m_layers.size()) {
        Layer *top = m_layers.at(index);
        Layer *bottom = m_layers.at(index - 1);

        QImage result = bottom->image().copy();
        QPainter painter(&result);
        painter.setOpacity(top->opacity());
        painter.drawImage(QPoint(0, 0), top->image());

        bottom->setImage(result);

        delete m_layers.takeAt(index);
        emit layerChanged();
    }
}

Layer *LayerStack::layer(int index) const
{
    if (index >= 0 && index < m_layers.size()) {
        return m_layers.at(index);
    }
    return nullptr;
}

QImage LayerStack::compositedImage() const
{
    if (m_layers.isEmpty()) return QImage();

    QImage result = m_layers.first()->image();

    for (int i = 1; i < m_layers.size(); ++i) {
        Layer *layer = m_layers.at(i);
        if (layer->visible()) {
            QPainter painter(&result);
            painter.setOpacity(layer->opacity());
            painter.drawImage(QPoint(0, 0), layer->image());
        }
    }

    return result;
}