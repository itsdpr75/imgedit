#include "operation.h"
#include <QPainter>
#include <QPainterPath>
#include <QJsonDocument>
#include <QJsonArray>
#include <QtMath>

Operation *Operation::fromJson(const QJsonObject &obj)
{
    QString type = obj.value("type").toString();
    if (type == "crop") {
        QJsonObject rectObj = obj.value("rect").toObject();
        QRect rect(rectObj.value("x").toInt(), rectObj.value("y").toInt(),
                   rectObj.value("width").toInt(), rectObj.value("height").toInt());
        return new CropOperation(rect);
    }
    if (type == "brush") {
        QJsonArray pointsArr = obj.value("points").toArray();
        QVector<BrushOperation::StrokePoint> points;
        for (int i = 0; i < pointsArr.size(); ++i) {
            QJsonObject ptObj = pointsArr[i].toObject();
            BrushOperation::StrokePoint pt;
            pt.position = QPointF(ptObj.value("x").toDouble(), ptObj.value("y").toDouble());
            pt.pressure = ptObj.value("pressure").toDouble(1.0);
            points.append(pt);
        }
        int size = obj.value("size").toInt();
        QColor color(obj.value("color").toString());
        qreal opacity = obj.value("opacity").toDouble(1.0);
        return new BrushOperation(points, size, color, opacity);
    }
    if (type == "blur") {
        QJsonArray pointsArr = obj.value("points").toArray();
        QVector<QPointF> points;
        for (int i = 0; i < pointsArr.size(); ++i) {
            QJsonObject ptObj = pointsArr[i].toObject();
            points.append(QPointF(ptObj.value("x").toDouble(), ptObj.value("y").toDouble()));
        }
        int radius = obj.value("radius").toInt();
        BlurOperation::Mode mode = obj.value("mode").toString() == "brush" ?
                                   BlurOperation::BrushMode : BlurOperation::RectangleMode;
        QRect rect;
        if (mode == BlurOperation::RectangleMode) {
            QJsonObject rectObj = obj.value("rect").toObject();
            rect = QRect(rectObj.value("x").toInt(), rectObj.value("y").toInt(),
                        rectObj.value("width").toInt(), rectObj.value("height").toInt());
        }
        return new BlurOperation(points, radius, mode, rect);
    }
    if (type == "pixelate") {
        QJsonArray pointsArr = obj.value("points").toArray();
        QVector<QPointF> points;
        for (int i = 0; i < pointsArr.size(); ++i) {
            QJsonObject ptObj = pointsArr[i].toObject();
            points.append(QPointF(ptObj.value("x").toDouble(), ptObj.value("y").toDouble()));
        }
        int pixelSize = obj.value("pixelSize").toInt();
        PixelateOperation::Mode mode = obj.value("mode").toString() == "brush" ?
                                       PixelateOperation::BrushMode : PixelateOperation::RectangleMode;
        QRect rect;
        if (mode == PixelateOperation::RectangleMode) {
            QJsonObject rectObj = obj.value("rect").toObject();
            rect = QRect(rectObj.value("x").toInt(), rectObj.value("y").toInt(),
                        rectObj.value("width").toInt(), rectObj.value("height").toInt());
        }
        return new PixelateOperation(points, pixelSize, mode, rect);
    }
    if (type == "text") {
        QString text = obj.value("text").toString();
        QJsonObject posObj = obj.value("position").toObject();
        QPointF position(posObj.value("x").toDouble(), posObj.value("y").toDouble());
        QString fontFamily = obj.value("fontFamily").toString();
        int fontSize = obj.value("fontSize").toInt();
        QColor color(obj.value("color").toString());
        qreal opacity = obj.value("opacity").toDouble(1.0);
        int rotation = obj.value("rotation").toInt();
        return new TextOperation(text, position, fontFamily, fontSize, color, opacity, rotation);
    }
    if (type == "filter") {
        QString filterName = obj.value("filterName").toString();
        QVariant value = obj.value("value").toVariant();
        return new FilterOperation(filterName, value);
    }
    return nullptr;
}

QJsonObject CropOperation::toJson() const
{
    QJsonObject obj;
    obj["type"] = "crop";
    obj["rect"] = QJsonObject({
        {"x", m_rect.x()},
        {"y", m_rect.y()},
        {"width", m_rect.width()},
        {"height", m_rect.height()}
    });
    return obj;
}

void CropOperation::apply(QImage &image)
{
    if (m_rect.isEmpty() || !m_rect.intersects(image.rect())) return;
    QImage *result = applyToImage(image);
    if (result) {
        image = result->copy();
        delete result;
    }
}

QImage *CropOperation::applyToImage(const QImage &base)
{
    return new QImage(base.copy(m_rect));
}

CropOperation::CropOperation(const QRect &rect) : m_rect(rect) {}

QJsonObject BrushOperation::toJson() const
{
    QJsonObject obj;
    obj["type"] = "brush";
    QJsonArray pointsArr;
    for (const auto &pt : m_points) {
        pointsArr.append(QJsonObject({
            {"x", pt.position.x()},
            {"y", pt.position.y()},
            {"pressure", pt.pressure}
        }));
    }
    obj["points"] = pointsArr;
    obj["size"] = m_size;
    obj["color"] = m_color.name(QColor::HexRgb);
    obj["opacity"] = m_opacity;
    return obj;
}

void BrushOperation::apply(QImage &image)
{
    QImage *result = applyToImage(image);
    image = result->copy();
    delete result;
}

QImage *BrushOperation::applyToImage(const QImage &base)
{
    QImage result = base.copy();
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen = painter.pen();
    pen.setColor(m_color);
    pen.setWidth(m_size);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    if (m_points.size() > 1) {
        QPainterPath path;
        path.moveTo(m_points[0].position);
        for (int i = 1; i < m_points.size(); ++i) {
            qreal pressure = m_points[i].pressure;
            int adjustedSize = qRound(m_size * (0.5 + 0.5 * pressure));
            pen.setWidth(adjustedSize);
            painter.setPen(pen);
            path.lineTo(m_points[i].position);
        }
        painter.drawPath(path);
    } else if (!m_points.isEmpty()) {
        painter.drawEllipse(m_points[0].position, m_size / 2.0, m_size / 2.0);
    }

    return new QImage(result);
}

BrushOperation::BrushOperation(const QVector<StrokePoint> &points, int size, const QColor &color, qreal opacity)
    : m_points(points), m_size(size), m_color(color), m_opacity(opacity)
{
    m_color.setAlphaF(opacity);
}

QJsonObject BlurOperation::toJson() const
{
    QJsonObject obj;
    obj["type"] = "blur";
    QJsonArray pointsArr;
    for (const auto &pt : m_points) {
        pointsArr.append(QJsonObject({
            {"x", pt.x()},
            {"y", pt.y()}
        }));
    }
    obj["points"] = pointsArr;
    obj["radius"] = m_radius;
    obj["mode"] = m_mode == BrushMode ? "brush" : "rectangle";
    if (m_mode == RectangleMode) {
        obj["rect"] = QJsonObject({
            {"x", m_rect.x()},
            {"y", m_rect.y()},
            {"width", m_rect.width()},
            {"height", m_rect.height()}
        });
    }
    return obj;
}

void BlurOperation::apply(QImage &image)
{
    QImage *result = applyToImage(image);
    image = result->copy();
    delete result;
}

QImage *BlurOperation::applyToImage(const QImage &base)
{
    QImage result = base.copy();

    if (m_mode == BrushMode && !m_points.isEmpty()) {
        for (const auto &pt : m_points) {
            QRectF rect(pt.x() - m_radius, pt.y() - m_radius, m_radius * 2, m_radius * 2);
            if (rect.intersects(result.rect())) {
                QImage region = result.copy(rect.toRect().intersected(result.rect()));
                if (!region.isNull()) {
                    QImage blurred = region.scaled(qMax(1, region.width() / 4), qMax(1, region.height() / 4),
                                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    blurred = blurred.scaled(region.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    QPainter p(&result);
                    p.drawImage(rect.topLeft(), blurred);
                }
            }
        }
    } else if (m_mode == RectangleMode && !m_rect.isEmpty()) {
        QRect rect = m_rect.intersected(result.rect());
        if (!rect.isEmpty()) {
            QImage region = result.copy(rect);
            QImage blurred = region.scaled(qMax(1, region.width() / 4), qMax(1, region.height() / 4),
                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            blurred = blurred.scaled(region.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QPainter p(&result);
            p.drawImage(rect.topLeft(), blurred);
        }
    }

    return new QImage(result);
}

BlurOperation::BlurOperation(const QVector<QPointF> &points, int radius, Mode mode, const QRect &rect)
    : m_points(points), m_radius(radius), m_mode(mode), m_rect(rect) {}

QJsonObject PixelateOperation::toJson() const
{
    QJsonObject obj;
    obj["type"] = "pixelate";
    QJsonArray pointsArr;
    for (const auto &pt : m_points) {
        pointsArr.append(QJsonObject({
            {"x", pt.x()},
            {"y", pt.y()}
        }));
    }
    obj["points"] = pointsArr;
    obj["pixelSize"] = m_pixelSize;
    obj["mode"] = m_mode == BrushMode ? "brush" : "rectangle";
    if (m_mode == RectangleMode) {
        obj["rect"] = QJsonObject({
            {"x", m_rect.x()},
            {"y", m_rect.y()},
            {"width", m_rect.width()},
            {"height", m_rect.height()}
        });
    }
    return obj;
}

void PixelateOperation::apply(QImage &image)
{
    QImage *result = applyToImage(image);
    image = result->copy();
    delete result;
}

static void pixelateRegionHelper(QImage &img, const QRect &rect, int pixelSize)
{
    for (int y = rect.top(); y < rect.bottom(); y += pixelSize) {
        for (int x = rect.left(); x < rect.right(); x += pixelSize) {
            int count = 0;
            int r = 0, g = 0, b = 0, a = 0;
            for (int dy = 0; dy < pixelSize && y + dy < rect.bottom(); ++dy) {
                for (int dx = 0; dx < pixelSize && x + dx < rect.right(); ++dx) {
                    QRgb pixel = img.pixel(x + dx, y + dy);
                    r += qRed(pixel);
                    g += qGreen(pixel);
                    b += qBlue(pixel);
                    a += qAlpha(pixel);
                    ++count;
                }
            }
            if (count > 0) {
                QRgb avg = qRgba(r / count, g / count, b / count, a / count);
                for (int dy = 0; dy < pixelSize && y + dy < rect.bottom(); ++dy) {
                    for (int dx = 0; dx < pixelSize && x + dx < rect.right(); ++dx) {
                        img.setPixel(x + dx, y + dy, avg);
                    }
                }
            }
        }
    }
}

QImage *PixelateOperation::applyToImage(const QImage &base)
{
    QImage result = base.copy();

    if (m_mode == BrushMode && !m_points.isEmpty()) {
        for (const auto &pt : m_points) {
            QRectF rect(pt.x() - m_pixelSize * 2, pt.y() - m_pixelSize * 2, m_pixelSize * 4, m_pixelSize * 4);
            QRect pixelRect = rect.toRect().intersected(result.rect());
            if (!pixelRect.isEmpty()) {
                pixelateRegionHelper(result, pixelRect, m_pixelSize);
            }
        }
    } else if (m_mode == RectangleMode && !m_rect.isEmpty()) {
        QRect rect = m_rect.intersected(result.rect());
        if (!rect.isEmpty()) {
            pixelateRegionHelper(result, rect, m_pixelSize);
        }
    }

    return new QImage(result);
}

PixelateOperation::PixelateOperation(const QVector<QPointF> &points, int pixelSize, Mode mode, const QRect &rect)
    : m_points(points), m_pixelSize(pixelSize), m_mode(mode), m_rect(rect) {}

QJsonObject TextOperation::toJson() const
{
    QJsonObject obj;
    obj["type"] = "text";
    obj["text"] = m_text;
    obj["position"] = QJsonObject({
        {"x", m_position.x()},
        {"y", m_position.y()}
    });
    obj["fontFamily"] = m_fontFamily;
    obj["fontSize"] = m_fontSize;
    obj["color"] = m_color.name(QColor::HexRgb);
    obj["opacity"] = m_opacity;
    obj["rotation"] = m_rotation;
    return obj;
}

void TextOperation::apply(QImage &image)
{
    QImage *result = applyToImage(image);
    image = result->copy();
    delete result;
}

QImage *TextOperation::applyToImage(const QImage &base)
{
    QImage result = base.copy();
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font(m_fontFamily, m_fontSize);
    font.setPixelSize(m_fontSize);
    painter.setFont(font);

    QColor textColor = m_color;
    textColor.setAlphaF(m_opacity);
    painter.setPen(textColor);

    painter.save();
    painter.translate(m_position);
    painter.rotate(m_rotation);
    painter.drawText(0, 0, m_text);
    painter.restore();

    return new QImage(result);
}

TextOperation::TextOperation(const QString &text, const QPointF &position,
                             const QString &fontFamily, int fontSize,
                             const QColor &color, qreal opacity, int rotation)
    : m_text(text), m_position(position), m_fontFamily(fontFamily),
      m_fontSize(fontSize), m_color(color), m_opacity(opacity), m_rotation(rotation) {}

QJsonObject FilterOperation::toJson() const
{
    QJsonObject obj;
    obj["type"] = "filter";
    obj["filterName"] = m_filterName;
    obj["value"] = QJsonValue::fromVariant(m_value);
    return obj;
}

void FilterOperation::apply(QImage &image)
{
    QImage *result = applyToImage(image);
    image = result->copy();
    delete result;
}

QImage *FilterOperation::applyToImage(const QImage &base)
{
    return new QImage(base);
}

FilterOperation::FilterOperation(const QString &filterName, const QVariant &value)
    : m_filterName(filterName), m_value(value) {}