#include "yolo_plugin.h"
#include "yolo_board.h"
#include <logos_api.h>
#include <QDebug>

YoloPlugin::YoloPlugin(QObject *parent)
    : QObject(parent)
    , m_board(new YoloBoard(this))
{}

void YoloPlugin::initLogos(LogosAPI *api) {
    m_logosAPI = api;
    logosAPI = api;
    if (!api) { qWarning() << "YoloPlugin: null LogosAPI"; return; }
#ifdef LOGOS_CORE_AVAILABLE
    auto *syncClient = api->getClient("sync_module");
    m_board->initWithClient(syncClient);
#endif
    qInfo() << "YoloPlugin: initialized v" << version();
    emit eventResponse("initialized", QVariantList() << "yolo" << version());
}

QString YoloPlugin::createBoard(const QString &name, const QString &creatorPubkey) {
    return m_board->createBoard(name, creatorPubkey);
}
QString YoloPlugin::postOpinion(const QString &prefix, const QString &authorPubkey,
                                const QString &title, const QString &content) {
    return m_board->postOpinion(prefix, authorPubkey, title, content);
}
QString YoloPlugin::getPosts(const QString &prefix) {
    return m_board->getPosts(prefix);
}
void YoloPlugin::followBoard(const QString &prefix) {
    m_board->followBoard(prefix);
}
