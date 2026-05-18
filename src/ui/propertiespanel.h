#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include <QWidget>
#include <QMap>

class QLabel;
class QSlider;
class QPushButton;
class QColorDialog;
class QFontComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QGroupBox;
class QRadioButton;

class PropertiesPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertiesPanel(QWidget *parent = nullptr);

    enum Mode {
        ModeNone,
        ModeBrush,
        ModeEraser,
        ModeText,
        ModeBlur,
        ModePixelate,
        ModeFilter
    };

    void setMode(Mode mode);
    Mode currentMode() const { return m_mode; }

    int brushSize() const;
    qreal brushOpacity() const;
    QColor brushColor() const;

    int blurRadius() const;
    int pixelSize() const;

    QString textFont() const;
    int textSize() const;
    int textRotation() const;
    QString text() const;

signals:
    void colorChanged(const QColor &color);
    void brushSizeChanged(int size);
    void brushOpacityChanged(int opacity);
    void blurRadiusChanged(int radius);
    void pixelSizeChanged(int size);
    void textFontChanged(const QString &font, int size);
    void textRotationChanged(int rotation);
    void textChanged(const QString &text);

    void filterBrightnessChanged(int value);
    void filterContrastChanged(int value);
    void filterSaturationChanged(int value);
    void filterHueChanged(int value);
    void filterTemperatureChanged(int value);
    void filterSharpnessChanged(int value);
    void filterHighlightsChanged(int value);
    void filterShadowsChanged(int value);

    void applyFiltersClicked();
    void resetFiltersClicked();

private slots:
    void onSliderValueChanged();
    void onColorButtonClicked();
    void onApplyFiltersClicked();
    void onResetFiltersClicked();

private:
    void createBrushPanel();
    void createTextPanel();
    void createBlurPanel();
    void createPixelatePanel();
    void createFilterPanel();

    void updateVisiblePanels();

    Mode m_mode = ModeNone;

    QGroupBox *m_brushGroup = nullptr;
    QSlider *m_brushSizeSlider = nullptr;
    QSlider *m_brushOpacitySlider = nullptr;
    QPushButton *m_colorButton = nullptr;
    QLabel *m_brushSizeLabel = nullptr;
    QLabel *m_brushOpacityLabel = nullptr;

    QGroupBox *m_textGroup = nullptr;
    QFontComboBox *m_fontCombo = nullptr;
    QSpinBox *m_fontSizeSpin = nullptr;
    QSpinBox *m_rotationSpin = nullptr;
    QPushButton *m_textColorButton = nullptr;
    QLabel *m_textColorLabel = nullptr;
    QLabel *m_fontLabel = nullptr;
    QLabel *m_sizeLabel = nullptr;
    QLabel *m_rotationLabel = nullptr;

    QGroupBox *m_blurGroup = nullptr;
    QSlider *m_blurRadiusSlider = nullptr;
    QLabel *m_blurRadiusLabel = nullptr;

    QGroupBox *m_pixelateGroup = nullptr;
    QSlider *m_pixelSizeSlider = nullptr;
    QLabel *m_pixelSizeLabel = nullptr;

    QGroupBox *m_filterGroup = nullptr;
    QMap<QString, QPair<QLabel*, QSlider*>> m_filterSliders;

    QPushButton *m_applyButton = nullptr;
    QPushButton *m_resetButton = nullptr;

    QColor m_currentColor;
    QColor m_textColor;
};

#endif