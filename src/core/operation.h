#ifndef OPERATION_H
#define OPERATION_H

#include <QImage>
#include <QJsonObject>
#include <QRectF>
#include <QPointF>
#include <QVector>
#include <QColor>
#include <QString>

class Operation
{
public:
    enum Type {
        TypeCrop,
        TypeBrush,
        TypeEraser,
        TypeBlur,
        TypePixelate,
        TypeText,
        TypeFilter
    };

    virtual ~Operation() = default;

    virtual Type type() const = 0;
    virtual QJsonObject toJson() const = 0;
    static Operation *fromJson(const QJsonObject &obj);

    virtual void apply(QImage &image) = 0;
    virtual QImage *applyToImage(const QImage &base) = 0;
};

class CropOperation : public Operation
{
public:
    explicit CropOperation(const QRect &rect);

    Type type() const override { return TypeCrop; }
    QJsonObject toJson() const override;
    void apply(QImage &image) override;
    QImage *applyToImage(const QImage &base) override;

    QRect cropRect() const { return m_rect; }
    void setCropRect(const QRect &rect) { m_rect = rect; }

private:
    QRect m_rect;
};

class BrushOperation : public Operation
{
public:
    struct StrokePoint {
        QPointF position;
        qreal pressure;
    };

    explicit BrushOperation(const QVector<StrokePoint> &points, int size, const QColor &color, qreal opacity);

    Type type() const override { return TypeBrush; }
    QJsonObject toJson() const override;
    void apply(QImage &image) override;
    QImage *applyToImage(const QImage &base) override;

    QVector<StrokePoint> points() const { return m_points; }
    int size() const { return m_size; }
    QColor color() const { return m_color; }
    qreal opacity() const { return m_opacity; }

private:
    QVector<StrokePoint> m_points;
    int m_size;
    QColor m_color;
    qreal m_opacity;
};

class BlurOperation : public Operation
{
public:
    enum Mode { BrushMode, RectangleMode };

    explicit BlurOperation(const QVector<QPointF> &points, int radius, Mode mode, const QRect &rect = QRect());

    Type type() const override { return TypeBlur; }
    QJsonObject toJson() const override;
    void apply(QImage &image) override;
    QImage *applyToImage(const QImage &base) override;

    int radius() const { return m_radius; }
    Mode mode() const { return m_mode; }
    QVector<QPointF> points() const { return m_points; }
    QRect regionRect() const { return m_rect; }

private:
    QVector<QPointF> m_points;
    int m_radius;
    Mode m_mode;
    QRect m_rect;
};

class PixelateOperation : public Operation
{
public:
    enum Mode { BrushMode, RectangleMode };

    explicit PixelateOperation(const QVector<QPointF> &points, int pixelSize, Mode mode, const QRect &rect = QRect());

    Type type() const override { return TypePixelate; }
    QJsonObject toJson() const override;
    void apply(QImage &image) override;
    QImage *applyToImage(const QImage &base) override;

    int pixelSize() const { return m_pixelSize; }
    Mode mode() const { return m_mode; }
    QVector<QPointF> points() const { return m_points; }
    QRect regionRect() const { return m_rect; }

private:
    QVector<QPointF> m_points;
    int m_pixelSize;
    Mode m_mode;
    QRect m_rect;
};

class TextOperation : public Operation
{
public:
    explicit TextOperation(const QString &text, const QPointF &position,
                          const QString &fontFamily, int fontSize,
                          const QColor &color, qreal opacity, int rotation);

    Type type() const override { return TypeText; }
    QJsonObject toJson() const override;
    void apply(QImage &image) override;
    QImage *applyToImage(const QImage &base) override;

    QString text() const { return m_text; }
    QPointF position() const { return m_position; }
    QString fontFamily() const { return m_fontFamily; }
    int fontSize() const { return m_fontSize; }
    QColor color() const { return m_color; }
    qreal opacity() const { return m_opacity; }
    int rotation() const { return m_rotation; }

    void setText(const QString &text) { m_text = text; }
    void setFontFamily(const QString &family) { m_fontFamily = family; }
    void setFontSize(int size) { m_fontSize = size; }
    void setColor(const QColor &color) { m_color = color; }
    void setOpacity(qreal opacity) { m_opacity = opacity; }
    void setRotation(int rotation) { m_rotation = rotation; }

private:
    QString m_text;
    QPointF m_position;
    QString m_fontFamily;
    int m_fontSize;
    QColor m_color;
    qreal m_opacity;
    int m_rotation;
};

class FilterOperation : public Operation
{
public:
    explicit FilterOperation(const QString &filterName, const QVariant &value);

    Type type() const override { return TypeFilter; }
    QJsonObject toJson() const override;
    void apply(QImage &image) override;
    QImage *applyToImage(const QImage &base) override;

    QString filterName() const { return m_filterName; }
    QVariant filterValue() const { return m_value; }
    void setFilterValue(const QVariant &value) { m_value = value; }

private:
    QString m_filterName;
    QVariant m_value;
};

#endif