#include "filter.h"
#include <QImage>
#include <QtMath>

Filter *Filter::fromName(const QString &name)
{
    if (name == "brightness") return new BrightnessFilter(0);
    if (name == "contrast") return new ContrastFilter(0);
    if (name == "saturation") return new SaturationFilter(0);
    if (name == "hue") return new HueFilter(0);
    if (name == "temperature") return new TemperatureFilter(0);
    if (name == "sharpness") return new SharpnessFilter(0);
    if (name == "highlights") return new HighlightsFilter(0);
    if (name == "shadows") return new ShadowsFilter(0);
    if (name == "blur") return new BlurFilter(10);
    if (name == "pixelate") return new PixelateFilter(10);
    return nullptr;
}

BrightnessFilter::BrightnessFilter(qreal value) : m_value(value) {}

void BrightnessFilter::apply(QImage &image)
{
    if (qFuzzyIsNull(m_value)) return;

    int brightness = qRound(m_value * 2.55);

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            int r = qBound(0, qRed(line[x]) + brightness, 255);
            int g = qBound(0, qGreen(line[x]) + brightness, 255);
            int b = qBound(0, qBlue(line[x]) + brightness, 255);
            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
}

ContrastFilter::ContrastFilter(qreal value) : m_value(value) {}

void ContrastFilter::apply(QImage &image)
{
    if (qFuzzyIsNull(m_value)) return;

    qreal factor = (100.0 + m_value) / 100.0;
    factor = factor * factor;

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            r = qBound(0, qRound((r / 255.0 - 0.5) * factor + 0.5) * 255, 255);
            g = qBound(0, qRound((g / 255.0 - 0.5) * factor + 0.5) * 255, 255);
            b = qBound(0, qRound((b / 255.0 - 0.5) * factor + 0.5) * 255, 255);

            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
}

SaturationFilter::SaturationFilter(qreal value) : m_value(value) {}

void SaturationFilter::apply(QImage &image)
{
    if (qFuzzyIsNull(m_value)) return;

    qreal saturation = 1.0 + m_value / 100.0;

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            qreal gray = 0.299 * r + 0.587 * g + 0.114 * b;

            r = qBound(0, qRound(gray + saturation * (r - gray)), 255);
            g = qBound(0, qRound(gray + saturation * (g - gray)), 255);
            b = qBound(0, qRound(gray + saturation * (b - gray)), 255);

            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
}

HueFilter::HueFilter(qreal value) : m_value(value) {}

void HueFilter::apply(QImage &image)
{
    if (qFuzzyIsNull(m_value)) return;

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            qreal h, s, l;
            QRgb rgb = line[x];
            qreal max = qMax(qreal(qRed(rgb)), qMax(qreal(qGreen(rgb)), qreal(qBlue(rgb))));
            qreal min = qMin(qreal(qRed(rgb)), qMin(qreal(qGreen(rgb)), qreal(qBlue(rgb))));
            l = (max + min) / 2.0 / 255.0;

            if (qFuzzyCompare(max, min)) {
                h = s = 0;
            } else {
                qreal d = max - min;
                s = l > 0.5 ? d / (2.0 - max - min) : d / (max + min);

                if (qFuzzyCompare(max, qreal(qRed(rgb)))) {
                    h = (qreal(qGreen(rgb)) - qreal(qBlue(rgb))) / d + (qGreen(rgb) < qBlue(rgb) ? 6 : 0);
                } else if (qFuzzyCompare(max, qreal(qGreen(rgb)))) {
                    h = (qreal(qBlue(rgb)) - qreal(qRed(rgb))) / d + 2;
                } else {
                    h = (qreal(qRed(rgb)) - qreal(qGreen(rgb))) / d + 4;
                }
                h /= 6;
            }

            h = h + m_value / 360.0;
            if (h > 1) h -= 1;
            if (h < 0) h += 1;

            qreal q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            qreal p = 2 * l - q;

            auto hue2rgb = [](qreal p, qreal q, qreal t) {
                if (t < 0) t += 1;
                if (t > 1) t -= 1;
                if (t < 1.0/6.0) return p + (q - p) * 6 * t;
                if (t < 1.0/2.0) return q;
                if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6;
                return p;
            };

            r = qRound(hue2rgb(p, q, h + 1.0/3.0) * 255);
            g = qRound(hue2rgb(p, q, h) * 255);
            b = qRound(hue2rgb(p, q, h - 1.0/3.0) * 255);

            line[x] = qRgba(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255), qAlpha(line[x]));
        }
    }
}

TemperatureFilter::TemperatureFilter(qreal value) : m_value(value) {}

void TemperatureFilter::apply(QImage &image)
{
    if (qFuzzyIsNull(m_value)) return;

    int warmth = qRound(m_value);

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            r = qBound(0, r + warmth, 255);
            b = qBound(0, b - warmth, 255);

            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
}

SharpnessFilter::SharpnessFilter(qreal value) : m_value(value) {}

void SharpnessFilter::apply(QImage &image)
{
    qDebug() << "[SharpnessFilter] START apply, value:" << m_value << "image size:" << image.size();
    
    if (m_value <= 0) {
        qDebug() << "[SharpnessFilter] exiting: m_value <= 0";
        return;
    }

    int w = image.width();
    int h = image.height();
    
    qDebug() << "[SharpnessFilter] Processing image of size" << w << "x" << h;

    qreal strength = m_value / 250.0;
    if (strength > 0.8) strength = 0.8;
    
    qDebug() << "[SharpnessFilter] strength:" << strength;

    QImage original = image.copy();
    
    for (int y = 1; y < h - 1; ++y) {
        QRgb *current = reinterpret_cast<QRgb *>(image.scanLine(y));
        QRgb *prev = reinterpret_cast<QRgb *>(original.scanLine(y - 1));
        QRgb *next = reinterpret_cast<QRgb *>(original.scanLine(y + 1));
        
        QRgb *prevRow = prev;
        QRgb *nextRow = next;

        for (int x = 1; x < w - 1; ++x) {
            int origR = qRed(current[x]);
            int origG = qGreen(current[x]);
            int origB = qBlue(current[x]);

            int neighborSumR = 
                qRed(prevRow[x-1]) + qRed(prevRow[x]) + qRed(prevRow[x+1]) +
                qRed(current[x-1]) + qRed(current[x+1]) +
                qRed(nextRow[x-1]) + qRed(nextRow[x]) + qRed(nextRow[x+1]);
            
            int neighborSumG = 
                qGreen(prevRow[x-1]) + qGreen(prevRow[x]) + qGreen(prevRow[x+1]) +
                qGreen(current[x-1]) + qGreen(current[x+1]) +
                qGreen(nextRow[x-1]) + qGreen(nextRow[x]) + qGreen(nextRow[x+1]);
            
            int neighborSumB = 
                qBlue(prevRow[x-1]) + qBlue(prevRow[x]) + qBlue(prevRow[x+1]) +
                qBlue(current[x-1]) + qBlue(current[x+1]) +
                qBlue(nextRow[x-1]) + qBlue(nextRow[x]) + qBlue(nextRow[x+1]);

            int edgeR = origR * 9 - neighborSumR;
            int edgeG = origG * 9 - neighborSumG;
            int edgeB = origB * 9 - neighborSumB;

            int finalR = qBound(0, qRound(origR + edgeR * strength), 255);
            int finalG = qBound(0, qRound(origG + edgeG * strength), 255);
            int finalB = qBound(0, qRound(origB + edgeB * strength), 255);

            current[x] = qRgba(finalR, finalG, finalB, qAlpha(current[x]));
        }
    }
    
    qDebug() << "[SharpnessFilter] DONE";
}

HighlightsFilter::HighlightsFilter(qreal value) : m_value(value) {}

void HighlightsFilter::apply(QImage &image)
{
    if (qFuzzyIsNull(m_value)) return;

    int adjustment = qRound(m_value * 2.55);

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            int brightness = (r + g + b) / 3;

            if (brightness > 128) {
                qreal factor = qreal(brightness - 128) / 127.0;
                int adjust = qRound(adjustment * factor);

                r = qBound(0, r + adjust, 255);
                g = qBound(0, g + adjust, 255);
                b = qBound(0, b + adjust, 255);
            }

            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
}

ShadowsFilter::ShadowsFilter(qreal value) : m_value(value) {}

void ShadowsFilter::apply(QImage &image)
{
    if (qFuzzyIsNull(m_value)) return;

    int adjustment = qRound(m_value * 2.55);

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            int brightness = (r + g + b) / 3;

            if (brightness < 64) {
                qreal factor = qreal(64 - brightness) / 64.0;
                int adjust = qRound(adjustment * factor);

                r = qBound(0, r + adjust, 255);
                g = qBound(0, g + adjust, 255);
                b = qBound(0, b + adjust, 255);
            }

            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
}

BlurFilter::BlurFilter(int radius) : m_radius(radius) {}

void BlurFilter::apply(QImage &image)
{
    if (m_radius <= 0) return;

    QImage src = image;
    int w = image.width();
    int h = image.height();

    for (int y = m_radius; y < h - m_radius; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));

        for (int x = m_radius; x < w - m_radius; ++x) {
            int r = 0, g = 0, b = 0, a = 0;
            int count = 0;

            for (int dy = -m_radius; dy <= m_radius; ++dy) {
                QRgb *srcLine = reinterpret_cast<QRgb *>(src.scanLine(y + dy));
                for (int dx = -m_radius; dx <= m_radius; ++dx) {
                    QRgb pixel = srcLine[x + dx];
                    r += qRed(pixel);
                    g += qGreen(pixel);
                    b += qBlue(pixel);
                    a += qAlpha(pixel);
                    ++count;
                }
            }

            line[x] = qRgba(r / count, g / count, b / count, a / count);
        }
    }
}

PixelateFilter::PixelateFilter(int pixelSize) : m_pixelSize(pixelSize) {}

void PixelateFilter::apply(QImage &image)
{
    if (m_pixelSize <= 1) return;

    int w = image.width();
    int h = image.height();

    for (int y = 0; y < h; y += m_pixelSize) {
        for (int x = 0; x < w; x += m_pixelSize) {
            int r = 0, g = 0, b = 0, a = 0;
            int count = 0;

            for (int dy = 0; dy < m_pixelSize && y + dy < h; ++dy) {
                for (int dx = 0; dx < m_pixelSize && x + dx < w; ++dx) {
                    QRgb pixel = image.pixel(x + dx, y + dy);
                    r += qRed(pixel);
                    g += qGreen(pixel);
                    b += qBlue(pixel);
                    a += qAlpha(pixel);
                    ++count;
                }
            }

            if (count > 0) {
                r /= count;
                g /= count;
                b /= count;
                a /= count;

                QRgb avg = qRgba(r, g, b, a);

                for (int dy = 0; dy < m_pixelSize && y + dy < h; ++dy) {
                    for (int dx = 0; dx < m_pixelSize && x + dx < w; ++dx) {
                        image.setPixel(x + dx, y + dy, avg);
                    }
                }
            }
        }
    }
}