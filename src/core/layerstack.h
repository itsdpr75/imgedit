#ifndef LAYERSTACK_H
#define LAYERSTACK_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QList>

class Layer
{
public:
    Layer(const QString &name = "Layer");
    ~Layer();

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QImage image() const { return m_image; }
    void setImage(const QImage &image) { m_image = image; }

    bool visible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity) { m_opacity = opacity; }

    int blendMode() const { return m_blendMode; }
    void setBlendMode(int mode) { m_blendMode = mode; }

private:
    QString m_name;
    QImage m_image;
    bool m_visible = true;
    qreal m_opacity = 1.0;
    int m_blendMode = 0;
};

class LayerStack : public QObject
{
    Q_OBJECT

public:
    explicit LayerStack(QObject *parent = nullptr);
    ~LayerStack();

    void addLayer(Layer *layer);
    void removeLayer(int index);
    void moveLayer(int from, int to);
    void mergeDown(int index);

    Layer *layer(int index) const;
    int layerCount() const { return m_layers.size(); }

    QImage compositedImage() const;

signals:
    void layerChanged();

private:
    QList<Layer*> m_layers;
};

#endif