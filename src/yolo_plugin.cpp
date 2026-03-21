#include "yolo_plugin.h"
#include "yolo_board.h"

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

    // Wire up module clients — following logos-pipe/src/sync_module.cpp pattern
    if (LogosAPIClient* blockchain = api->getClient("blockchain_module")) {
        m_module->setBlockchainClient(blockchain);
        qInfo() << "YoloPlugin: blockchain_module client connected";
    } else {
        qWarning() << "YoloPlugin: blockchain_module not available";
    }

    if (LogosAPIClient* kv = api->getClient("kv_module")) {
        m_module->setKvClient(kv);
        qInfo() << "YoloPlugin: kv_module client connected";
    } else {
        qWarning() << "YoloPlugin: kv_module not available (caching disabled)";
    }

    if (LogosAPIClient* storage = api->getClient("storage_module")) {
        m_module->setStorageClient(storage);
        qInfo() << "YoloPlugin: storage_module client connected";
    } else {
        qWarning() << "YoloPlugin: storage_module not available (large posts disabled)";
    }

    qInfo() << "YoloPlugin: initLogos done. version:" << version();
}

QString YoloPlugin::hello() const {
    return m_module->hello();
}
