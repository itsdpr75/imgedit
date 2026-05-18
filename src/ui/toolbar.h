#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QButtonGroup>

class QToolButton;

class Toolbar : public QToolBar
{
    Q_OBJECT

public:
    explicit Toolbar(QWidget *parent = nullptr);

    enum ToolAction {
        ActionSelect,
        ActionCrop,
        ActionBrush,
        ActionEraser,
        ActionBlur,
        ActionPixelate,
        ActionText
    };

    void setCurrentTool(ToolAction tool);
    ToolAction currentTool() const { return m_currentTool; }

signals:
    void toolSelected(ToolAction tool);

private:
    void createActions();
    void updateCheckedState();

    ToolAction m_currentTool = ActionSelect;
    QButtonGroup *m_toolGroup = nullptr;
    QAction *m_actSelect = nullptr;
    QAction *m_actCrop = nullptr;
    QAction *m_actBrush = nullptr;
    QAction *m_actEraser = nullptr;
    QAction *m_actBlur = nullptr;
    QAction *m_actPixelate = nullptr;
    QAction *m_actText = nullptr;
};

#endif