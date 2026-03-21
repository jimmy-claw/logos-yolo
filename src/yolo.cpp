#include "yolo.h"

#include <QDebug>

Yolo::Yolo(QObject *parent)
    : QObject(parent)
{
    qDebug() << "Yolo: initialized";
}

QString Yolo::hello() const {
    return QStringLiteral("Hello from Yolo!");
}

void Yolo::initLogos(LogosAPI *logosAPIInstance) {
#ifdef LOGOS_CORE_AVAILABLE
    Q_UNUSED(logosAPIInstance)
    qDebug() << "Yolo: initLogos called";
#else
    Q_UNUSED(logosAPIInstance)
#endif
}
