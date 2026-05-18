#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileSystemWatcher>

class CanvasView;
class Toolbar;
class PropertiesPanel;
class ImageDocument;
class Tool;
class Filter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openFile(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveProjectAs();
    void onExportImage();
    void onUndo();
    void onRedo();
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onZoom100();
    void onToolChanged(int toolType);
    void onFilterChanged(int filterType, const QVariant &value);
    void onColorChanged(const QColor &color);
    void onBrushSizeChanged(int size);
    void onBrushOpacityChanged(int opacity);
    void onTextFontChanged(const QString &fontFamily, int fontSize);
    void onTextRotationChanged(int rotation);
    void onBlurRadiusChanged(int radius);
    void onPixelSizeChanged(int size);
    void onCropApply();
    void onCropCancel();
    void onApplyFilter();
    void onResetFilters();
    void onDocumentModified();

private:
    void createActions();
    void createMenus();
    void createToolbars();
    void createPanels();
    void setupConnections();

    bool saveProjectPrompt();
    bool maybeSave();

    QString currentFilePath;
    bool isProjectModified = false;

    CanvasView *m_canvasView = nullptr;
    Toolbar *m_toolbar = nullptr;
    PropertiesPanel *m_propertiesPanel = nullptr;
    ImageDocument *m_document = nullptr;
    Tool *m_currentTool = nullptr;

    QAction *m_actionUndo = nullptr;
    QAction *m_actionRedo = nullptr;
    QAction *m_actionSave = nullptr;
};

#endif