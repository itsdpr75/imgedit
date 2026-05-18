#include "projectfile.h"
#include "../core/imagedocument.h"
#include "../core/operation.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QImage>

bool ProjectFile::save(ImageDocument *document, const QString &path)
{
    QJsonObject json = documentToJson(document);
    QJsonDocument doc(json);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ProjectFile::load(ImageDocument *document, const QString &path)
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

    return jsonToDocument(doc.object(), document);
}

QJsonObject ProjectFile::documentToJson(ImageDocument *document)
{
    QJsonObject json;
    json["version"] = "1.0";

    if (!document->originalImagePath().isEmpty()) {
        json["originalImage"] = document->originalImagePath();
    }

    QImage image = document->originalImage();
    if (!image.isNull()) {
        QByteArray thumbData;
        QBuffer buffer(&thumbData);
        image.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation)
             .save(&buffer, "PNG");
        json["thumbnail"] = QString::fromUtf8(thumbData.toBase64());
    }

    json["filters"] = QJsonObject();

    return json;
}

bool ProjectFile::jsonToDocument(const QJsonObject &json, ImageDocument *document)
{
    QString version = json.value("version").toString();

    QString imagePath = json.value("originalImage").toString();
    if (!imagePath.isEmpty()) {
        document->loadImage(imagePath);
    }

    return true;
}

QString ProjectFile::imageToBase64Thumbnail(const QImage &image, int maxSize)
{
    if (image.isNull()) return QString();

    QImage thumb = image.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QByteArray data;
    QBuffer buffer(&data);
    thumb.save(&buffer, "PNG");

    return QString::fromUtf8(data.toBase64());
}