#include "toolbar.h"
#include "../utils/fontawesome.h"
#include <QToolButton>
#include <QIcon>
#include <QPainter>
#include <QActionGroup>

static QIcon createFAIcon(FontAwesome::Icon icon)
{
    QIcon ico;
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setPen(Qt::white);
    QFont font = FontAwesome::instance().font(24);
    p.setFont(font);
    p.drawText(QRect(0, 0, 32, 32), Qt::AlignCenter, QString(FontAwesome::instance().icon(icon)));
    ico.addPixmap(pm);
    return ico;
}

Toolbar::Toolbar(QWidget *parent)
    : QToolBar(parent)
{
    setMovable(false);
    setFloatable(false);
    setIconSize(QSize(24, 24));
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    createActions();
}

void Toolbar::createActions()
{
    m_actSelect = new QAction(createFAIcon(FontAwesome::IconMousePointer), tr("Select"), this);
    m_actSelect->setToolTip(tr("Select (V)"));

    m_actCrop = new QAction(createFAIcon(FontAwesome::IconCrop), tr("Crop"), this);
    m_actCrop->setToolTip(tr("Crop (C)"));

    m_actBrush = new QAction(createFAIcon(FontAwesome::IconPaintbrush), tr("Brush"), this);
    m_actBrush->setToolTip(tr("Brush (B)"));

    m_actEraser = new QAction(createFAIcon(FontAwesome::IconEraser), tr("Eraser"), this);
    m_actEraser->setToolTip(tr("Eraser (E)"));

    m_actBlur = new QAction(createFAIcon(FontAwesome::IconTools), tr("Blur"), this);
    m_actBlur->setToolTip(tr("Blur (U)"));

    m_actPixelate = new QAction(createFAIcon(FontAwesome::IconImage), tr("Pixelate"), this);
    m_actPixelate->setToolTip(tr("Pixelate (P)"));

    m_actText = new QAction(createFAIcon(FontAwesome::IconFont), tr("Text"), this);
    m_actText->setToolTip(tr("Text (T)"));

    m_toolGroup = new QButtonGroup(this);
    m_toolGroup->setExclusive(true);

    QList<QAction*> actions = {m_actSelect, m_actCrop, m_actBrush, m_actEraser,
                              m_actBlur, m_actPixelate, m_actText};

    QList<ToolAction> toolActions = {ActionSelect, ActionCrop, ActionBrush, ActionEraser,
                                     ActionBlur, ActionPixelate, ActionText};

    for (int i = 0; i < actions.size(); ++i) {
        actions[i]->setCheckable(true);
        
        QToolButton *btn = new QToolButton(this);
        btn->setDefaultAction(actions[i]);
        btn->setToolTip(actions[i]->toolTip());
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        m_toolGroup->addButton(btn, i);
        
        connect(actions[i], &QAction::toggled, this, [this, toolActions, i](bool checked) {
            if (checked) {
                emit toolSelected(toolActions[i]);
            }
        });
        
        addWidget(btn);
    }

    m_actSelect->setChecked(true);
}

void Toolbar::setCurrentTool(ToolAction tool)
{
    if (m_currentTool == tool) return;

    m_currentTool = tool;
    updateCheckedState();
    emit toolSelected(tool);
}

void Toolbar::updateCheckedState()
{
    QList<QAction*> actions = {m_actSelect, m_actCrop, m_actBrush, m_actEraser,
                              m_actBlur, m_actPixelate, m_actText};

    for (int i = 0; i < actions.size(); ++i) {
        actions[i]->setChecked(i == m_currentTool);
    }
}