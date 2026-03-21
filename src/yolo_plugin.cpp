#include "yolo_plugin.h"

#include <QDebug>

YoloPlugin::YoloPlugin(QObject *parent)
    : QObject(parent)
    , m_module(new Yolo(this))
{
    connect(m_module, &Yolo::eventResponse,
            this, &YoloPlugin::eventResponse);
}

void YoloPlugin::initLogos(LogosAPI *api) {
    m_logosAPI = api;
    logosAPI = api;  // PluginInterface base-class field

    if (!m_logosAPI) {
        qWarning() << "YoloPlugin: initLogos called with null LogosAPI";
        return;
    }

    qInfo() << "YoloPlugin: initLogos done. version:" << version();
}

QString YoloPlugin::hello() const {
    return m_module->hello();
}
