#ifndef ICONENGINE_H
#define ICONENGINE_H

#include <QIconEngine>
#include <QString>

class IconEngine : public QIconEngine
{
public:
    IconEngine(const QString &iconName, const QString &fontFamily, int size = 16);
    ~IconEngine();

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;

private:
    QString m_iconName;
    QString m_fontFamily;
    int m_size;
};

#endif