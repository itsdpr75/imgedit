#ifndef FILTER_H
#define FILTER_H

#include <QString>
#include <QVariant>
#include <QImage>

class Filter
{
public:
    virtual ~Filter() = default;

    virtual QString name() const = 0;
    virtual void apply(QImage &image) = 0;
    virtual Filter *clone() const = 0;

    static Filter *fromName(const QString &name);
};

class BrightnessFilter : public Filter
{
public:
    explicit BrightnessFilter(qreal value);
    QString name() const override { return "brightness"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new BrightnessFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class ContrastFilter : public Filter
{
public:
    explicit ContrastFilter(qreal value);
    QString name() const override { return "contrast"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new ContrastFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class SaturationFilter : public Filter
{
public:
    explicit SaturationFilter(qreal value);
    QString name() const override { return "saturation"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new SaturationFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class HueFilter : public Filter
{
public:
    explicit HueFilter(qreal value);
    QString name() const override { return "hue"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new HueFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class TemperatureFilter : public Filter
{
public:
    explicit TemperatureFilter(qreal value);
    QString name() const override { return "temperature"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new TemperatureFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class SharpnessFilter : public Filter
{
public:
    explicit SharpnessFilter(qreal value);
    QString name() const override { return "sharpness"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new SharpnessFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class HighlightsFilter : public Filter
{
public:
    explicit HighlightsFilter(qreal value);
    QString name() const override { return "highlights"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new HighlightsFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class ShadowsFilter : public Filter
{
public:
    explicit ShadowsFilter(qreal value);
    QString name() const override { return "shadows"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new ShadowsFilter(m_value); }
    qreal value() const { return m_value; }

private:
    qreal m_value;
};

class BlurFilter : public Filter
{
public:
    explicit BlurFilter(int radius);
    QString name() const override { return "blur"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new BlurFilter(m_radius); }
    int radius() const { return m_radius; }

private:
    int m_radius;
};

class PixelateFilter : public Filter
{
public:
    explicit PixelateFilter(int pixelSize);
    QString name() const override { return "pixelate"; }
    void apply(QImage &image) override;
    Filter *clone() const override { return new PixelateFilter(m_pixelSize); }
    int pixelSize() const { return m_pixelSize; }

private:
    int m_pixelSize;
};

#endif