#include "colorpicker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QColorDialog>

ColorPicker::ColorPicker(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    m_colorButton = new QPushButton("", this);
    m_colorButton->setFixedSize(40, 24);
    layout->addWidget(m_colorButton);

    m_opacityLabel = new QLabel("100%", this);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setMinimum(0);
    m_opacitySlider->setMaximum(100);
    m_opacitySlider->setValue(100);
    layout->addWidget(m_opacityLabel);
    layout->addWidget(m_opacitySlider);

    connect(m_colorButton, &QPushButton::clicked, this, &ColorPicker::onColorButtonClicked);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &ColorPicker::onOpacityChanged);

    setColor(Qt::red);
}

void ColorPicker::setColor(const QColor &color)
{
    m_color = color;
    m_colorButton->setStyleSheet(QString("background-color: %1; border: 1px solid #555;").arg(color.name()));
}

void ColorPicker::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_color, this, tr("Select Color"));
    if (color.isValid()) {
        setColor(color);
        emit colorChanged(color);
    }
}

void ColorPicker::onOpacityChanged(int value)
{
    m_opacityLabel->setText(QString("%1%").arg(value));
    m_color.setAlpha(value * 255 / 100);
    emit colorChanged(m_color);
}