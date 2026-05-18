#ifndef IMAGEDOCUMENT_H
#define IMAGEDOCUMENT_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QRect>
#include <QColor>
#include <QMap>
#include <QVariant>

class Operation;
class UndoStack;
class Filter;

class ImageDocument : public QObject
{
    Q_OBJECT

public:
    explicit ImageDocument(QObject *parent = nullptr);
    ~ImageDocument();

    bool loadImage(const QString &path);
    bool loadProject(const QString &path);
    bool saveProject(const QString &path);
    bool exportImage(const QString &path, const QString &format = "PNG");

    QImage originalImage() const { return m_originalImage; }
    QImage currentImage() const { return m_currentImage; }
    QImage renderedImage() const;

    QString originalImagePath() const { return m_originalImagePath; }

    void setOriginalImage(const QImage &image);

    void applyOperation(Operation *op);
    void undo();
    void redo();

    UndoStack *undoStack() const { return m_undoStack; }

    bool hasImage() const { return !m_originalImage.isNull(); }

    void setFilterValue(const QString &filterName, const QVariant &value);
    QVariant filterValue(const QString &filterName) const;

    void resetFilters();
    void applyAllFilters();

    void clear();

    bool isModified() const { return m_isModified; }
    void setModified(bool modified);

    QString projectFilePath() const { return m_projectFilePath; }
    void setProjectFilePath(const QString &path);

signals:
    void imageChanged(const QImage &image);
    void undoStackChanged();
    void filterChanged(const QString &name, const QVariant &value);
    void filtersReset();
    void modifiedChanged(bool modified);

private:
    void rebuildCurrentImage();

    QImage m_originalImage;
    QImage m_currentImage;
    QString m_originalImagePath;
    QString m_projectFilePath;

    UndoStack *m_undoStack = nullptr;

    QMap<QString, QVariant> m_filterValues;

    bool m_isModified = false;
    bool m_filtersApplied = false;
};

#endif