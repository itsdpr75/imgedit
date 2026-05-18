#include "mainwindow.h"
#include "ui/canvasview.h"
#include "ui/toolbar.h"
#include "ui/propertiespanel.h"
#include "core/imagedocument.h"
#include "core/undostack.h"
#include "tools/tool.h"
#include "utils/fontawesome.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QCloseEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QInputDialog>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileInfo>
#include <QDockWidget>

static QIcon createFAIcon(FontAwesome::Icon icon)
{
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    QFont font = FontAwesome::instance().font(24);
    p.setFont(font);
    p.setPen(Qt::white);
    p.drawText(QRect(0, 0, 32, 32), Qt::AlignCenter, QString(FontAwesome::instance().icon(icon)));
    return QIcon(pm);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("imgedit");
    setMinimumSize(800, 600);
    resize(1200, 800);

    setAcceptDrops(true);

    m_document = new ImageDocument(this);

    m_canvasView = new CanvasView(this);
    m_canvasView->setDocument(m_document);
    setCentralWidget(m_canvasView);

    m_toolbar = new Toolbar(this);
    addToolBar(Qt::LeftToolBarArea, m_toolbar);

    m_propertiesPanel = new PropertiesPanel(this);
    QDockWidget *dock = new QDockWidget(tr("Properties"), this);
    dock->setWidget(m_propertiesPanel);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    createActions();
    createMenus();
    setupConnections();

    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {}

void MainWindow::openFile(const QString &path)
{
    if (path.endsWith(".imgedit", Qt::CaseInsensitive)) {
        if (m_document->loadProject(path)) {
            currentFilePath = path;
            setWindowTitle("imgedit - " + path);
            m_canvasView->zoomFit();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open project file."));
        }
    } else {
        if (m_document->loadImage(path)) {
            setWindowTitle("imgedit - " + path);
            m_canvasView->zoomFit();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open image file."));
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!maybeSave()) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().first();
        QString path = url.toLocalFile();
        if (!path.isEmpty()) {
            openFile(path);
        }
    }
}

void MainWindow::onNewProject()
{
    if (!maybeSave()) return;

    QString filter = "Image Files (*.png *.jpg *.jpeg *.bmp *.gif *.tiff)";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), filter);

    if (!fileName.isEmpty()) {
        if (m_document->loadImage(fileName)) {
            currentFilePath.clear();
            setWindowTitle("imgedit - " + fileName);
            m_canvasView->zoomFit();
            m_actionSave->setEnabled(true);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open image file."));
        }
    }
}

void MainWindow::onOpenProject()
{
    if (!maybeSave()) return;

    QString filter = "Image Files (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;imgedit Projects (*.imgedit)";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), QString(), filter);

    if (!fileName.isEmpty()) {
        openFile(fileName);
    }
}

void MainWindow::onSaveProject()
{
    if (currentFilePath.isEmpty()) {
        onSaveProjectAs();
    } else {
        m_document->saveProject(currentFilePath);
    }
}

void MainWindow::onSaveProjectAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Project"),
                                                     QString(), "imgedit Projects (*.imgedit)");
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".imgedit", Qt::CaseInsensitive)) {
            fileName += ".imgedit";
        }
        m_document->saveProject(fileName);
        currentFilePath = fileName;
        setWindowTitle("imgedit - " + fileName);
    }
}

void MainWindow::onExportImage()
{
    QString filter = "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;TIFF (*.tiff)";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Image"),
                                                     QString(), filter);
    if (!fileName.isEmpty()) {
        QString format = QFileInfo(fileName).suffix().toUpper();
        if (format.isEmpty()) {
            format = "PNG";
            fileName += ".png";
        }
        m_document->exportImage(fileName, format);
    }
}

void MainWindow::onUndo()
{
    m_document->undo();
}

void MainWindow::onRedo()
{
    m_document->redo();
}

void MainWindow::onZoomIn()
{
    m_canvasView->zoomIn();
    statusBar()->showMessage(tr("Zoom: %1%").arg(int(m_canvasView->zoom() * 100)));
}

void MainWindow::onZoomOut()
{
    m_canvasView->zoomOut();
    statusBar()->showMessage(tr("Zoom: %1%").arg(int(m_canvasView->zoom() * 100)));
}

void MainWindow::onZoomFit()
{
    m_canvasView->zoomFit();
    statusBar()->showMessage(tr("Zoom: Fit"));
}

void MainWindow::onZoom100()
{
    m_canvasView->zoom100();
    statusBar()->showMessage(tr("Zoom: 100%"));
}

void MainWindow::onToolChanged(int toolType)
{
    if (m_currentTool) {
        delete m_currentTool;
        m_currentTool = nullptr;
    }

    switch (toolType) {
    case Toolbar::ActionSelect:
        m_currentTool = nullptr;
        m_canvasView->setCursor(Qt::ArrowCursor);
        m_propertiesPanel->setMode(PropertiesPanel::ModeNone);
        break;
    case Toolbar::ActionCrop:
        m_currentTool = new CropTool(this);
        m_propertiesPanel->setMode(PropertiesPanel::ModeNone);
        break;
    case Toolbar::ActionBrush:
        m_currentTool = new BrushTool(this);
        m_currentTool->setColor(m_propertiesPanel->brushColor());
        m_currentTool->setBrushSize(m_propertiesPanel->brushSize());
        m_currentTool->setOpacity(m_propertiesPanel->brushOpacity());
        m_propertiesPanel->setMode(PropertiesPanel::ModeBrush);
        break;
    case Toolbar::ActionEraser:
        m_currentTool = new EraserTool(this);
        m_currentTool->setBrushSize(m_propertiesPanel->brushSize());
        m_propertiesPanel->setMode(PropertiesPanel::ModeEraser);
        break;
    case Toolbar::ActionBlur:
        m_currentTool = new BlurTool(this);
        m_currentTool->setBrushSize(m_propertiesPanel->brushSize());
        dynamic_cast<BlurTool*>(m_currentTool)->setRadius(m_propertiesPanel->blurRadius());
        m_propertiesPanel->setMode(PropertiesPanel::ModeBlur);
        break;
    case Toolbar::ActionPixelate:
        m_currentTool = new PixelateTool(this);
        m_currentTool->setBrushSize(m_propertiesPanel->brushSize());
        dynamic_cast<PixelateTool*>(m_currentTool)->setPixelSize(m_propertiesPanel->pixelSize());
        m_propertiesPanel->setMode(PropertiesPanel::ModePixelate);
        break;
    case Toolbar::ActionText:
        m_currentTool = new TextTool(this);
        dynamic_cast<TextTool*>(m_currentTool)->setColor(m_propertiesPanel->brushColor());
        dynamic_cast<TextTool*>(m_currentTool)->setFontFamily(m_propertiesPanel->textFont());
        dynamic_cast<TextTool*>(m_currentTool)->setFontSize(m_propertiesPanel->textSize());
        dynamic_cast<TextTool*>(m_currentTool)->setRotation(m_propertiesPanel->textRotation());
        dynamic_cast<TextTool*>(m_currentTool)->setOpacity(1.0);
        m_propertiesPanel->setMode(PropertiesPanel::ModeText);
        break;
    }

    m_canvasView->setTool(m_currentTool);

    if (toolType == Toolbar::ActionSelect) {
        m_propertiesPanel->setMode(PropertiesPanel::ModeNone);
    } else if (toolType == Toolbar::ActionCrop) {
        m_propertiesPanel->setMode(PropertiesPanel::ModeNone);
    }
}

void MainWindow::onFilterChanged(int filterType, const QVariant &value)
{
    Q_UNUSED(filterType);
    QString name;
    switch (filterType) {
        case 0: name = "brightness"; break;
        case 1: name = "contrast"; break;
        case 2: name = "saturation"; break;
        case 3: name = "hue"; break;
        case 4: name = "temperature"; break;
        case 5: name = "sharpness"; break;
        case 6: name = "highlights"; break;
        case 7: name = "shadows"; break;
    }
    m_document->setFilterValue(name, value);
}

void MainWindow::onColorChanged(const QColor &color)
{
    if (m_currentTool) {
        m_currentTool->setColor(color);
    }
}

void MainWindow::onBrushSizeChanged(int size)
{
    if (m_currentTool) {
        m_currentTool->setBrushSize(size);
    }
}

void MainWindow::onBrushOpacityChanged(int opacity)
{
    if (m_currentTool) {
        m_currentTool->setOpacity(opacity / 100.0);
    }
}

void MainWindow::onTextFontChanged(const QString &fontFamily, int fontSize)
{
    if (m_currentTool && m_currentTool->type() == ToolText) {
        TextTool *textTool = dynamic_cast<TextTool*>(m_currentTool);
        textTool->setFontFamily(fontFamily);
        textTool->setFontSize(fontSize);
    }
}

void MainWindow::onTextRotationChanged(int rotation)
{
    if (m_currentTool && m_currentTool->type() == ToolText) {
        dynamic_cast<TextTool*>(m_currentTool)->setRotation(rotation);
    }
}

void MainWindow::onBlurRadiusChanged(int radius)
{
    if (m_currentTool && m_currentTool->type() == ToolBlur) {
        dynamic_cast<BlurTool*>(m_currentTool)->setRadius(radius);
    }
}

void MainWindow::onPixelSizeChanged(int size)
{
    if (m_currentTool && m_currentTool->type() == ToolPixelate) {
        dynamic_cast<PixelateTool*>(m_currentTool)->setPixelSize(size);
    }
}

void MainWindow::onCropApply()
{
    if (m_currentTool && m_currentTool->type() == ToolCrop) {
        CropTool *cropTool = dynamic_cast<CropTool*>(m_currentTool);
        Operation *op = cropTool->commitOperation(m_canvasView);
        if (op) {
            m_document->applyOperation(op);
        }
    }
}

void MainWindow::onCropCancel()
{
    if (m_currentTool && m_currentTool->type() == ToolCrop) {
        dynamic_cast<CropTool*>(m_currentTool)->reset();
        m_canvasView->clearCropOverlay();
    }
}

void MainWindow::onApplyFilter()
{
    m_document->applyAllFilters();
}

void MainWindow::onResetFilters()
{
    m_document->resetFilters();
}

void MainWindow::onDocumentModified()
{
    isProjectModified = true;
    QString title = "imgedit";
    if (!currentFilePath.isEmpty()) {
        title += " - " + currentFilePath;
    }
    if (isProjectModified) {
        title += " *";
    }
    setWindowTitle(title);
    m_actionSave->setEnabled(true);
}

bool MainWindow::saveProjectPrompt()
{
    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Save Project"),
        tr("Do you want to save the current project?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        onSaveProject();
        return true;
    } else if (ret == QMessageBox::Discard) {
        return true;
    }
    return false;
}

bool MainWindow::maybeSave()
{
    if (!m_document->isModified()) return true;
    return saveProjectPrompt();
}

void MainWindow::createActions()
{
    m_actionUndo = new QAction(createFAIcon(FontAwesome::IconUndo), tr("Undo"), this);
    m_actionUndo->setShortcut(QKeySequence::Undo);
    m_actionUndo->setEnabled(false);
    connect(m_actionUndo, &QAction::triggered, this, &MainWindow::onUndo);

    m_actionRedo = new QAction(createFAIcon(FontAwesome::IconRedo), tr("Redo"), this);
    m_actionRedo->setShortcut(QKeySequence::Redo);
    m_actionRedo->setEnabled(false);
    connect(m_actionRedo, &QAction::triggered, this, &MainWindow::onRedo);

    m_actionSave = new QAction(createFAIcon(FontAwesome::IconSave), tr("Save"), this);
    m_actionSave->setShortcut(QKeySequence::Save);
    m_actionSave->setEnabled(false);
    connect(m_actionSave, &QAction::triggered, this, &MainWindow::onSaveProject);

    QAction *actionNew = new QAction(createFAIcon(FontAwesome::IconFile), tr("New"), this);
    actionNew->setShortcut(QKeySequence::New);
    connect(actionNew, &QAction::triggered, this, &MainWindow::onNewProject);

    QAction *actionOpen = new QAction(createFAIcon(FontAwesome::IconFolderOpen), tr("Open..."), this);
    actionOpen->setShortcut(QKeySequence::Open);
    connect(actionOpen, &QAction::triggered, this, &MainWindow::onOpenProject);

    QAction *actionSaveAs = new QAction(tr("Save As..."), this);
    actionSaveAs->setShortcut(QKeySequence::SaveAs);
    connect(actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveProjectAs);

    QAction *actionExport = new QAction(createFAIcon(FontAwesome::IconExport), tr("Export Image..."), this);
    actionExport->setShortcut(QKeySequence::SaveAs | Qt::ShiftModifier);
    connect(actionExport, &QAction::triggered, this, &MainWindow::onExportImage);

    QAction *actionQuit = new QAction(tr("Quit"), this);
    actionQuit->setShortcut(QKeySequence::Quit);
    connect(actionQuit, &QAction::triggered, this, &QWidget::close);

    QAction *actionZoomIn = new QAction(createFAIcon(FontAwesome::IconZoomIn), tr("Zoom In"), this);
    actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    connect(actionZoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);

    QAction *actionZoomOut = new QAction(createFAIcon(FontAwesome::IconZoomOut), tr("Zoom Out"), this);
    actionZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(actionZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);

    QAction *actionZoomFit = new QAction(createFAIcon(FontAwesome::IconCompress), tr("Fit to Window"), this);
    actionZoomFit->setShortcut(Qt::Key_0);
    connect(actionZoomFit, &QAction::triggered, this, &MainWindow::onZoomFit);

    QAction *actionZoom100 = new QAction(tr("Actual Size"), this);
    actionZoom100->setShortcut(Qt::Key_1);
    connect(actionZoom100, &QAction::triggered, this, &MainWindow::onZoom100);

    QAction *actionFilterPanel = new QAction(tr("Filters"), this);
    actionFilterPanel->setCheckable(true);
    actionFilterPanel->setChecked(true);
    connect(actionFilterPanel, &QAction::toggled, this, [this](bool checked) {
        m_propertiesPanel->setVisible(checked);
    });

    m_toolbar->addAction(actionNew);
    m_toolbar->addAction(actionOpen);
    m_toolbar->addAction(m_actionSave);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actionUndo);
    m_toolbar->addAction(m_actionRedo);
    m_toolbar->addSeparator();
    m_toolbar->addAction(actionZoomIn);
    m_toolbar->addAction(actionZoomOut);
    m_toolbar->addAction(actionZoomFit);

    connect(m_toolbar, &Toolbar::toolSelected, this, [this](Toolbar::ToolAction tool) {
        onToolChanged(tool);
    });

    connect(m_propertiesPanel, &PropertiesPanel::colorChanged, this, &MainWindow::onColorChanged);
    connect(m_propertiesPanel, &PropertiesPanel::brushSizeChanged, this, &MainWindow::onBrushSizeChanged);
    connect(m_propertiesPanel, &PropertiesPanel::brushOpacityChanged, this, &MainWindow::onBrushOpacityChanged);
    connect(m_propertiesPanel, &PropertiesPanel::blurRadiusChanged, this, &MainWindow::onBlurRadiusChanged);
    connect(m_propertiesPanel, &PropertiesPanel::pixelSizeChanged, this, &MainWindow::onPixelSizeChanged);
    connect(m_propertiesPanel, &PropertiesPanel::textFontChanged, this, &MainWindow::onTextFontChanged);
    connect(m_propertiesPanel, &PropertiesPanel::textRotationChanged, this, &MainWindow::onTextRotationChanged);
    connect(m_propertiesPanel, &PropertiesPanel::applyFiltersClicked, this, &MainWindow::onApplyFilter);
    connect(m_propertiesPanel, &PropertiesPanel::resetFiltersClicked, this, &MainWindow::onResetFilters);

    connect(m_propertiesPanel, &PropertiesPanel::filterBrightnessChanged, this, [this](int value) {
        qDebug() << "[FILTER] Brightness changed to:" << value;
        m_document->setFilterValue("brightness", value);
    });
    connect(m_propertiesPanel, &PropertiesPanel::filterContrastChanged, this, [this](int value) {
        qDebug() << "[FILTER] Contrast changed to:" << value;
        m_document->setFilterValue("contrast", value);
    });
    connect(m_propertiesPanel, &PropertiesPanel::filterSaturationChanged, this, [this](int value) {
        qDebug() << "[FILTER] Saturation changed to:" << value;
        m_document->setFilterValue("saturation", value);
    });
    connect(m_propertiesPanel, &PropertiesPanel::filterHueChanged, this, [this](int value) {
        qDebug() << "[FILTER] Hue changed to:" << value;
        m_document->setFilterValue("hue", value);
    });
    connect(m_propertiesPanel, &PropertiesPanel::filterTemperatureChanged, this, [this](int value) {
        qDebug() << "[FILTER] Temperature changed to:" << value;
        m_document->setFilterValue("temperature", value);
    });
    connect(m_propertiesPanel, &PropertiesPanel::filterSharpnessChanged, this, [this](int value) {
        qDebug() << "[FILTER] Sharpness changed to:" << value;
        m_document->setFilterValue("sharpness", value);
    });
    connect(m_propertiesPanel, &PropertiesPanel::filterHighlightsChanged, this, [this](int value) {
        qDebug() << "[FILTER] Highlights changed to:" << value;
        m_document->setFilterValue("highlights", value);
    });
    connect(m_propertiesPanel, &PropertiesPanel::filterShadowsChanged, this, [this](int value) {
        qDebug() << "[FILTER] Shadows changed to:" << value;
        m_document->setFilterValue("shadows", value);
    });

    connect(m_document->undoStack(), &UndoStack::canUndoChanged, m_actionUndo, &QAction::setEnabled);
    connect(m_document->undoStack(), &UndoStack::canRedoChanged, m_actionRedo, &QAction::setEnabled);
    connect(m_document, &ImageDocument::modifiedChanged, this, &MainWindow::onDocumentModified);

    connect(m_canvasView, &CanvasView::toolApplied, this, [this](Operation *op) {
        m_document->applyOperation(op);
        m_canvasView->clearBrushPreview();
        m_canvasView->clearCropOverlay();
        m_canvasView->clearEffectOverlay();
    });

    m_propertiesPanel->setMode(PropertiesPanel::ModeFilter);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("New"), this, &MainWindow::onNewProject, QKeySequence::New);
    fileMenu->addAction(tr("Open..."), this, &MainWindow::onOpenProject, QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Save"), this, &MainWindow::onSaveProject, QKeySequence::Save);
    fileMenu->addAction(tr("Save As..."), this, &MainWindow::onSaveProjectAs, QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Export Image..."), this, &MainWindow::onExportImage);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Quit"), this, &QWidget::close, QKeySequence::Quit);

    QMenu *editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(m_actionUndo);
    editMenu->addAction(m_actionRedo);
    editMenu->addSeparator();
    editMenu->addAction(m_actionSave);

    QMenu *viewMenu = menuBar()->addMenu(tr("View"));
    viewMenu->addAction(tr("Zoom In"), this, &MainWindow::onZoomIn, QKeySequence::ZoomIn);
    viewMenu->addAction(tr("Zoom Out"), this, &MainWindow::onZoomOut, QKeySequence::ZoomOut);
    viewMenu->addAction(tr("Fit to Window"), this, &MainWindow::onZoomFit, Qt::Key_0);
    viewMenu->addAction(tr("Actual Size"), this, &MainWindow::onZoom100, Qt::Key_1);

    QMenu *toolsMenu = menuBar()->addMenu(tr("Tools"));
    toolsMenu->addAction(tr("Crop"), this, [this]() { onToolChanged(Toolbar::ActionCrop); }, Qt::Key_C);
    toolsMenu->addAction(tr("Brush"), this, [this]() { onToolChanged(Toolbar::ActionBrush); }, Qt::Key_B);
    toolsMenu->addAction(tr("Eraser"), this, [this]() { onToolChanged(Toolbar::ActionEraser); }, Qt::Key_E);
    toolsMenu->addAction(tr("Blur"), this, [this]() { onToolChanged(Toolbar::ActionBlur); }, Qt::Key_U);
    toolsMenu->addAction(tr("Pixelate"), this, [this]() { onToolChanged(Toolbar::ActionPixelate); }, Qt::Key_P);
    toolsMenu->addAction(tr("Text"), this, [this]() { onToolChanged(Toolbar::ActionText); }, Qt::Key_T);

    QMenu *filtersMenu = menuBar()->addMenu(tr("Adjustments"));
    filtersMenu->addAction(tr("Apply Filters"), this, &MainWindow::onApplyFilter);
    filtersMenu->addAction(tr("Reset Filters"), this, &MainWindow::onResetFilters);
}

void MainWindow::createToolbars()
{
}

void MainWindow::createPanels()
{
}

void MainWindow::setupConnections()
{
}