#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QColor>

class QPushButton;
class QLabel;
class QSlider;

class ColorPicker : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPicker(QWidget *parent = nullptr);

    QColor currentColor() const { return m_color; }
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

private slots:
    void onColorButtonClicked();
    void onOpacityChanged(int value);

private:
    QColor m_color;
    QPushButton *m_colorButton = nullptr;
    QLabel *m_opacityLabel = nullptr;
    QSlider *m_opacitySlider = nullptr;
};

#endif