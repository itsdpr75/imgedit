#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include <QString>
#include <QJsonObject>

class ImageDocument;

class ProjectFile
{
public:
    static bool save(ImageDocument *document, const QString &path);
    static bool load(ImageDocument *document, const QString &path);

    static QJsonObject documentToJson(ImageDocument *document);
    static bool jsonToDocument(const QJsonObject &json, ImageDocument *document);

private:
    static QString imageToBase64Thumbnail(const QImage &image, int maxSize = 256);
};

#endif