#include "imagedocument.h"
#include "undostack.h"
#include "operation.h"
#include "../filters/filter.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QDir>
#include <QBuffer>
#include <QDebug>

ImageDocument::ImageDocument(QObject *parent)
    : QObject(parent)
    , m_undoStack(new UndoStack(this))
{
    m_filterValues["brightness"] = 0;
    m_filterValues["contrast"] = 0;
    m_filterValues["saturation"] = 0;
    m_filterValues["hue"] = 0;
    m_filterValues["temperature"] = 0;
    m_filterValues["sharpness"] = 0;
    m_filterValues["highlights"] = 0;
    m_filterValues["shadows"] = 0;

    connect(m_undoStack, &UndoStack::stateChanged, this, &ImageDocument::undoStackChanged);
}

ImageDocument::~ImageDocument()
{
    clear();
}

bool ImageDocument::loadImage(const QString &path)
{
    clear();

    QImage image(path);
    if (image.isNull()) {
        return false;
    }

    m_originalImage = image.convertToFormat(QImage::Format_ARGB32);
    m_originalImagePath = path;
    m_currentImage = m_originalImage;

    emit imageChanged(m_currentImage);
    return true;
}

bool ImageDocument::loadProject(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    QString version = root.value("version").toString();

    QString imagePath = root.value("originalImage").toString();
    if (!imagePath.isEmpty()) {
        if (!loadImage(imagePath)) {
            QString thumbnail = root.value("thumbnail").toString();
            if (!thumbnail.isEmpty()) {
                QByteArray thumbData = QByteArray::fromBase64(thumbnail.toUtf8());
                m_originalImage = QImage::fromData(thumbData);
                m_originalImagePath = "";
            }
        }
    }

    QJsonArray operations = root.value("operations").toArray();
    for (const auto &opVal : operations) {
        QJsonObject opObj = opVal.toObject();
        Operation *op = Operation::fromJson(opObj);
        if (op) {
            applyOperation(op);
        }
    }

    QJsonObject filters = root.value("filters").toObject();
    for (auto it = filters.begin(); it != filters.end(); ++it) {
        m_filterValues[it.key()] = it.value().toVariant();
    }

    m_projectFilePath = path;
    m_isModified = false;
    emit modifiedChanged(false);

    return true;
}

bool ImageDocument::saveProject(const QString &path)
{
    QJsonObject root;
    root["version"] = "1.0";
    root["originalImage"] = m_originalImagePath;

    if (!m_originalImagePath.isEmpty()) {
        QByteArray thumbData;
        QBuffer buffer(&thumbData);
        m_originalImage.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                       .save(&buffer, "PNG");
        root["thumbnail"] = QString::fromUtf8(thumbData.toBase64());
    }

    root["operations"] = QJsonArray();

    QJsonObject filters;
    for (auto it = m_filterValues.begin(); it != m_filterValues.end(); ++it) {
        filters[it.key()] = QJsonValue::fromVariant(it.value());
    }
    root["filters"] = filters;

    QJsonDocument doc(root);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    m_projectFilePath = path;
    m_isModified = false;
    emit modifiedChanged(false);

    return true;
}

bool ImageDocument::exportImage(const QString &path, const QString &format)
{
    QImage exportImg = renderedImage();
    if (exportImg.isNull()) {
        return false;
    }
    return exportImg.save(path, format.toUtf8().constData());
}

void ImageDocument::setOriginalImage(const QImage &image)
{
    clear();
    m_originalImage = image.convertToFormat(QImage::Format_ARGB32);
    m_currentImage = m_originalImage;
    m_originalImagePath = "";
    emit imageChanged(m_currentImage);
}

void ImageDocument::applyOperation(Operation *op)
{
    QImage result = m_currentImage;
    op->apply(result);
    m_currentImage = result;
    m_undoStack->push(op);
    m_isModified = true;
    emit imageChanged(m_currentImage);
    emit modifiedChanged(true);
}

void ImageDocument::undo()
{
    if (!m_undoStack->canUndo()) return;

    m_undoStack->undo();
    rebuildCurrentImage();
    m_isModified = true;
    emit imageChanged(m_currentImage);
    emit modifiedChanged(true);
}

void ImageDocument::redo()
{
    if (!m_undoStack->canRedo()) return;

    m_undoStack->redo();
    rebuildCurrentImage();
    m_isModified = true;
    emit imageChanged(m_currentImage);
    emit modifiedChanged(true);
}

void ImageDocument::setFilterValue(const QString &filterName, const QVariant &value)
{
    qDebug() << "[ImageDocument] setFilterValue:" << filterName << "=" << value;
    m_filterValues[filterName] = value;
    m_filtersApplied = true;
    rebuildCurrentImage();
    qDebug() << "[ImageDocument] emitting imageChanged, current image isNull:" << m_currentImage.isNull();
    emit imageChanged(m_currentImage);
    emit filterChanged(filterName, value);
}

QVariant ImageDocument::filterValue(const QString &filterName) const
{
    return m_filterValues.value(filterName);
}

void ImageDocument::resetFilters()
{
    m_filterValues["brightness"] = 0;
    m_filterValues["contrast"] = 0;
    m_filterValues["saturation"] = 0;
    m_filterValues["hue"] = 0;
    m_filterValues["temperature"] = 0;
    m_filterValues["sharpness"] = 0;
    m_filterValues["highlights"] = 0;
    m_filterValues["shadows"] = 0;
    m_filtersApplied = false;
    rebuildCurrentImage();
    emit imageChanged(m_currentImage);
    emit filtersReset();
}

void ImageDocument::applyAllFilters()
{
    m_filtersApplied = true;
    rebuildCurrentImage();
    emit imageChanged(m_currentImage);
}

QImage ImageDocument::renderedImage() const
{
    if (m_currentImage.isNull()) {
        return m_originalImage;
    }

    if (m_filtersApplied) {
        QImage result = m_currentImage;

        qreal brightness = m_filterValues["brightness"].toReal();
        qreal contrast = m_filterValues["contrast"].toReal();
        qreal saturation = m_filterValues["saturation"].toReal();
        qreal hue = m_filterValues["hue"].toReal();
        qreal temperature = m_filterValues["temperature"].toReal();
        qreal sharpness = m_filterValues["sharpness"].toReal();
        qreal highlights = m_filterValues["highlights"].toReal();
        qreal shadows = m_filterValues["shadows"].toReal();

        if (brightness != 0) {
            BrightnessFilter filter(brightness);
            filter.apply(result);
        }
        if (contrast != 0) {
            ContrastFilter filter(contrast);
            filter.apply(result);
        }
        if (saturation != 0) {
            SaturationFilter filter(saturation);
            filter.apply(result);
        }
        if (hue != 0) {
            HueFilter filter(hue);
            filter.apply(result);
        }
        if (temperature != 0) {
            TemperatureFilter filter(temperature);
            filter.apply(result);
        }
        if (sharpness != 0) {
            SharpnessFilter filter(sharpness);
            filter.apply(result);
        }
        if (highlights != 0) {
            HighlightsFilter filter(highlights);
            filter.apply(result);
        }
        if (shadows != 0) {
            ShadowsFilter filter(shadows);
            filter.apply(result);
        }

        return result;
    }

    return m_currentImage;
}

void ImageDocument::rebuildCurrentImage()
{
    m_currentImage = m_originalImage;

    if (m_filtersApplied) {
        qDebug() << "[ImageDocument] Applying filters to currentImage";

        qreal brightness = m_filterValues["brightness"].toReal();
        qreal contrast = m_filterValues["contrast"].toReal();
        qreal saturation = m_filterValues["saturation"].toReal();
        qreal hue = m_filterValues["hue"].toReal();
        qreal temperature = m_filterValues["temperature"].toReal();
        qreal sharpness = m_filterValues["sharpness"].toReal();
        qreal highlights = m_filterValues["highlights"].toReal();
        qreal shadows = m_filterValues["shadows"].toReal();

        if (brightness != 0) {
            BrightnessFilter filter(brightness);
            filter.apply(m_currentImage);
        }
        if (contrast != 0) {
            ContrastFilter filter(contrast);
            filter.apply(m_currentImage);
        }
        if (saturation != 0) {
            SaturationFilter filter(saturation);
            filter.apply(m_currentImage);
        }
        if (hue != 0) {
            HueFilter filter(hue);
            filter.apply(m_currentImage);
        }
        if (temperature != 0) {
            TemperatureFilter filter(temperature);
            filter.apply(m_currentImage);
        }
        if (sharpness != 0) {
            SharpnessFilter filter(sharpness);
            filter.apply(m_currentImage);
        }
        if (highlights != 0) {
            HighlightsFilter filter(highlights);
            filter.apply(m_currentImage);
        }
        if (shadows != 0) {
            ShadowsFilter filter(shadows);
            filter.apply(m_currentImage);
        }
    }
}

void ImageDocument::clear()
{
    m_originalImage = QImage();
    m_currentImage = QImage();
    m_originalImagePath = "";
    m_undoStack->clear();
    m_filterValues["brightness"] = 0;
    m_filterValues["contrast"] = 0;
    m_filterValues["saturation"] = 0;
    m_filterValues["hue"] = 0;
    m_filterValues["temperature"] = 0;
    m_filterValues["sharpness"] = 0;
    m_filterValues["highlights"] = 0;
    m_filterValues["shadows"] = 0;
    m_filtersApplied = false;
    m_isModified = false;
    emit imageChanged(QImage());
}

void ImageDocument::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit modifiedChanged(modified);
    }
}

void ImageDocument::setProjectFilePath(const QString &path)
{
    m_projectFilePath = path;
}