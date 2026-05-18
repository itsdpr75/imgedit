#include "propertiespanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QFontComboBox>
#include <QSpinBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QDebug>

PropertiesPanel::PropertiesPanel(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(250);
    setMaximumWidth(300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    createBrushPanel();
    createTextPanel();
    createBlurPanel();
    createPixelatePanel();
    createFilterPanel();

    mainLayout->addStretch();

    m_currentColor = Qt::red;
    m_textColor = Qt::white;

    setMode(ModeNone);
}

void PropertiesPanel::createBrushPanel()
{
    m_brushGroup = new QGroupBox(tr("Brush"), this);
    QVBoxLayout *layout = new QVBoxLayout(m_brushGroup);

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    m_brushSizeLabel = new QLabel(tr("Size: 10"), this);
    m_brushSizeSlider = new QSlider(Qt::Horizontal, this);
    m_brushSizeSlider->setMinimum(1);
    m_brushSizeSlider->setMaximum(100);
    m_brushSizeSlider->setValue(10);
    sizeLayout->addWidget(m_brushSizeLabel);
    sizeLayout->addWidget(m_brushSizeSlider);
    layout->addLayout(sizeLayout);

    QHBoxLayout *opacityLayout = new QHBoxLayout;
    m_brushOpacityLabel = new QLabel(tr("Opacity: 100%"), this);
    m_brushOpacitySlider = new QSlider(Qt::Horizontal, this);
    m_brushOpacitySlider->setMinimum(0);
    m_brushOpacitySlider->setMaximum(100);
    m_brushOpacitySlider->setValue(100);
    opacityLayout->addWidget(m_brushOpacityLabel);
    opacityLayout->addWidget(m_brushOpacitySlider);
    layout->addLayout(opacityLayout);

    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLayout->addWidget(new QLabel(tr("Color:"), this));
    m_colorButton = new QPushButton("", this);
    m_colorButton->setFixedSize(40, 24);
    m_colorButton->setStyleSheet("background-color: red; border: 1px solid #555;");
    colorLayout->addWidget(m_colorButton);
    colorLayout->addStretch();
    layout->addLayout(colorLayout);

    connect(m_brushSizeSlider, &QSlider::valueChanged, this, &PropertiesPanel::onSliderValueChanged);
    connect(m_brushOpacitySlider, &QSlider::valueChanged, this, &PropertiesPanel::onSliderValueChanged);
    connect(m_colorButton, &QPushButton::clicked, this, &PropertiesPanel::onColorButtonClicked);

    static_cast<QVBoxLayout*>(this->layout())->addWidget(m_brushGroup);
}

void PropertiesPanel::createTextPanel()
{
    m_textGroup = new QGroupBox(tr("Text"), this);
    QVBoxLayout *layout = new QVBoxLayout(m_textGroup);

    QHBoxLayout *fontLayout = new QHBoxLayout;
    m_fontLabel = new QLabel(tr("Font:"), this);
    m_fontCombo = new QFontComboBox(this);
    m_fontCombo->setCurrentText("Sans");
    fontLayout->addWidget(m_fontLabel);
    fontLayout->addWidget(m_fontCombo);
    layout->addLayout(fontLayout);

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    m_sizeLabel = new QLabel(tr("Size:"), this);
    m_fontSizeSpin = new QSpinBox(this);
    m_fontSizeSpin->setMinimum(8);
    m_fontSizeSpin->setMaximum(200);
    m_fontSizeSpin->setValue(24);
    sizeLayout->addWidget(m_sizeLabel);
    sizeLayout->addWidget(m_fontSizeSpin);
    layout->addLayout(sizeLayout);

    QHBoxLayout *rotationLayout = new QHBoxLayout;
    m_rotationLabel = new QLabel(tr("Rotation:"), this);
    m_rotationSpin = new QSpinBox(this);
    m_rotationSpin->setMinimum(-180);
    m_rotationSpin->setMaximum(180);
    m_rotationSpin->setValue(0);
    rotationLayout->addWidget(m_rotationLabel);
    rotationLayout->addWidget(m_rotationSpin);
    layout->addLayout(rotationLayout);

    QHBoxLayout *colorLayout = new QHBoxLayout;
    m_textColorLabel = new QLabel(tr("Color:"), this);
    m_textColorButton = new QPushButton("", this);
    m_textColorButton->setFixedSize(40, 24);
    m_textColorButton->setStyleSheet("background-color: white; border: 1px solid #555;");
    colorLayout->addWidget(m_textColorLabel);
    colorLayout->addWidget(m_textColorButton);
    colorLayout->addStretch();
    layout->addLayout(colorLayout);

    connect(m_fontCombo, &QFontComboBox::currentTextChanged, this, [this]() {
        emit textFontChanged(m_fontCombo->currentText(), m_fontSizeSpin->value());
    });
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
        emit textFontChanged(m_fontCombo->currentText(), m_fontSizeSpin->value());
    });
    connect(m_rotationSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::textRotationChanged);
    connect(m_textColorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(m_textColor, this, tr("Select Text Color"));
        if (color.isValid()) {
            m_textColor = color;
            m_textColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid #555;")
                                              .arg(color.name()));
            emit colorChanged(color);
        }
    });

    static_cast<QVBoxLayout*>(this->layout())->addWidget(m_textGroup);
}

void PropertiesPanel::createBlurPanel()
{
    m_blurGroup = new QGroupBox(tr("Blur"), this);
    QVBoxLayout *layout = new QVBoxLayout(m_blurGroup);

    QHBoxLayout *radiusLayout = new QHBoxLayout;
    m_blurRadiusLabel = new QLabel(tr("Radius: 20"), this);
    m_blurRadiusSlider = new QSlider(Qt::Horizontal, this);
    m_blurRadiusSlider->setMinimum(1);
    m_blurRadiusSlider->setMaximum(50);
    m_blurRadiusSlider->setValue(20);
    radiusLayout->addWidget(m_blurRadiusLabel);
    radiusLayout->addWidget(m_blurRadiusSlider);
    layout->addLayout(radiusLayout);

    connect(m_blurRadiusSlider, &QSlider::valueChanged, this, &PropertiesPanel::onSliderValueChanged);

    static_cast<QVBoxLayout*>(this->layout())->addWidget(m_blurGroup);
}

void PropertiesPanel::createPixelatePanel()
{
    m_pixelateGroup = new QGroupBox(tr("Pixelate"), this);
    QVBoxLayout *layout = new QVBoxLayout(m_pixelateGroup);

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    m_pixelSizeLabel = new QLabel(tr("Pixel Size: 10"), this);
    m_pixelSizeSlider = new QSlider(Qt::Horizontal, this);
    m_pixelSizeSlider->setMinimum(2);
    m_pixelSizeSlider->setMaximum(50);
    m_pixelSizeSlider->setValue(10);
    sizeLayout->addWidget(m_pixelSizeLabel);
    sizeLayout->addWidget(m_pixelSizeSlider);
    layout->addLayout(sizeLayout);

    connect(m_pixelSizeSlider, &QSlider::valueChanged, this, &PropertiesPanel::onSliderValueChanged);

    static_cast<QVBoxLayout*>(this->layout())->addWidget(m_pixelateGroup);
}

void PropertiesPanel::createFilterPanel()
{
    m_filterGroup = new QGroupBox(tr("Adjustments"), this);
    QVBoxLayout *layout = new QVBoxLayout(m_filterGroup);

    auto addSlider = [&](const QString &name, int min, int max, int defaultValue) {
        QHBoxLayout *hLayout = new QHBoxLayout;
        QLabel *label = new QLabel(name, this);
        label->setMinimumWidth(70);
        QSlider *slider = new QSlider(Qt::Horizontal, this);
        slider->setMinimum(min);
        slider->setMaximum(max);
        slider->setValue(defaultValue);
        hLayout->addWidget(label);
        hLayout->addWidget(slider);
        layout->addLayout(hLayout);

        m_filterSliders[name] = qMakePair(label, slider);
        connect(slider, &QSlider::valueChanged, this, &PropertiesPanel::onSliderValueChanged);
    };

    addSlider("Brightness", -100, 100, 0);
    addSlider("Contrast", -100, 100, 0);
    addSlider("Saturation", -100, 100, 0);
    addSlider("Hue", -180, 180, 0);
    addSlider("Temperature", -100, 100, 0);
    addSlider("Sharpness", 0, 200, 0);
    addSlider("Highlights", -100, 100, 0);
    addSlider("Shadows", -100, 100, 0);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_applyButton = new QPushButton(tr("Apply"), this);
    m_resetButton = new QPushButton(tr("Reset"), this);
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_resetButton);
    layout->addLayout(buttonLayout);

    connect(m_applyButton, &QPushButton::clicked, this, &PropertiesPanel::onApplyFiltersClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &PropertiesPanel::onResetFiltersClicked);

    static_cast<QVBoxLayout*>(this->layout())->addWidget(m_filterGroup);
}

void PropertiesPanel::setMode(Mode mode)
{
    m_mode = mode;
    updateVisiblePanels();
}

void PropertiesPanel::updateVisiblePanels()
{
    m_brushGroup->setVisible(m_mode == ModeBrush || m_mode == ModeEraser);
    m_textGroup->setVisible(m_mode == ModeText);
    m_blurGroup->setVisible(m_mode == ModeBlur);
    m_pixelateGroup->setVisible(m_mode == ModePixelate);
    m_filterGroup->setVisible(true);

    if (m_mode == ModeEraser) {
        m_colorButton->hide();
    } else if (m_mode != ModeText) {
        m_colorButton->show();
    }
}

int PropertiesPanel::brushSize() const
{
    return m_brushSizeSlider->value();
}

qreal PropertiesPanel::brushOpacity() const
{
    return m_brushOpacitySlider->value() / 100.0;
}

QColor PropertiesPanel::brushColor() const
{
    if (m_mode == ModeEraser) {
        return QColor(0, 0, 0, 0);
    }
    return m_currentColor;
}

int PropertiesPanel::blurRadius() const
{
    return m_blurRadiusSlider->value();
}

int PropertiesPanel::pixelSize() const
{
    return m_pixelSizeSlider->value();
}

QString PropertiesPanel::textFont() const
{
    return m_fontCombo->currentText();
}

int PropertiesPanel::textSize() const
{
    return m_fontSizeSpin->value();
}

int PropertiesPanel::textRotation() const
{
    return m_rotationSpin->value();
}

QString PropertiesPanel::text() const
{
    return "";
}

void PropertiesPanel::onSliderValueChanged()
{
    qDebug() << "[PropertiesPanel] Slider changed, sender:" << sender();

    if (sender() == m_brushSizeSlider) {
        m_brushSizeLabel->setText(tr("Size: %1").arg(m_brushSizeSlider->value()));
        emit brushSizeChanged(m_brushSizeSlider->value());
    } else if (sender() == m_brushOpacitySlider) {
        m_brushOpacityLabel->setText(tr("Opacity: %1%").arg(m_brushOpacitySlider->value()));
        emit brushOpacityChanged(m_brushOpacitySlider->value());
    } else if (sender() == m_blurRadiusSlider) {
        m_blurRadiusLabel->setText(tr("Radius: %1").arg(m_blurRadiusSlider->value()));
        emit blurRadiusChanged(m_blurRadiusSlider->value());
    } else if (sender() == m_pixelSizeSlider) {
        m_pixelSizeLabel->setText(tr("Pixel Size: %1").arg(m_pixelSizeSlider->value()));
        emit pixelSizeChanged(m_pixelSizeSlider->value());
    } else {
        qDebug() << "[PropertiesPanel] Checking filter sliders, count:" << m_filterSliders.size();
        for (auto it = m_filterSliders.begin(); it != m_filterSliders.end(); ++it) {
            qDebug() << "[PropertiesPanel] Checking slider:" << it.key() << "slider:" << it.value().second << "sender match:" << (sender() == it.value().second);
            if (sender() == it.value().second) {
                QString name = it.key();
                int value = it.value().second->value();
                qDebug() << "[PropertiesPanel] Emitting filter signal for:" << name << "value:" << value;
                if (name == "Brightness") emit filterBrightnessChanged(value);
                else if (name == "Contrast") emit filterContrastChanged(value);
                else if (name == "Saturation") emit filterSaturationChanged(value);
                else if (name == "Hue") emit filterHueChanged(value);
                else if (name == "Temperature") emit filterTemperatureChanged(value);
                else if (name == "Sharpness") emit filterSharpnessChanged(value);
                else if (name == "Highlights") emit filterHighlightsChanged(value);
                else if (name == "Shadows") emit filterShadowsChanged(value);
                break;
            }
        }
    }
}

void PropertiesPanel::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_currentColor, this, tr("Select Brush Color"));
    if (color.isValid()) {
        m_currentColor = color;
        m_colorButton->setStyleSheet(QString("background-color: %1; border: 1px solid #555;")
                                      .arg(color.name()));
        emit colorChanged(color);
    }
}

void PropertiesPanel::onApplyFiltersClicked()
{
    emit applyFiltersClicked();
}

void PropertiesPanel::onResetFiltersClicked()
{
    for (auto it = m_filterSliders.begin(); it != m_filterSliders.end(); ++it) {
        it.value().second->setValue(0);
    }
    emit resetFiltersClicked();
}