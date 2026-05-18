#include "iconengine.h"
#include <QPainter>
#include <QFont>

IconEngine::IconEngine(const QString &iconName, const QString &fontFamily, int size)
    : m_iconName(iconName), m_fontFamily(fontFamily), m_size(size) {}

IconEngine::~IconEngine() {}

void IconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);

    QFont font(m_fontFamily);
    font.setPixelSize(rect.height() * 0.7);
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignCenter, m_iconName);
}

QIconEngine *IconEngine::clone() const
{
    return new IconEngine(m_iconName, m_fontFamily, m_size);
}