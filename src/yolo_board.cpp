#include "yolo_board.h"

#ifdef LOGOS_CORE_AVAILABLE
#include <logos_api_client.h>
#endif

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>

YoloBoard::YoloBoard(QObject *parent) : QObject(parent) {}

#ifdef LOGOS_CORE_AVAILABLE
void YoloBoard::setSyncClient(LogosAPIClient *client) {
    m_sync = client;
}
#endif

QString YoloBoard::makeBoardPrefix(const QString &creatorPubkey) {
    return QStringLiteral("YOLO:%1:%2")
        .arg(creatorPubkey.left(8))
        .arg(QDateTime::currentSecsSinceEpoch());
}

QString YoloBoard::createBoard(const QString &name, const QString &creatorPubkey) {
    const QString prefix = makeBoardPrefix(creatorPubkey);
    QJsonObject result;
    result["success"] = true;
    result["prefix"] = prefix;
    result["name"] = name;

#ifdef LOGOS_CORE_AVAILABLE
    if (m_sync) {
        m_sync->invokeRemoteMethod("sync_module", "federatedAddAdmin", prefix, creatorPubkey);
    }
#endif

    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

QString YoloBoard::postOpinion(const QString &prefix, const QString &authorPubkey,
                               const QString &title, const QString &content) {
    QJsonObject post;
    post["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    post["author"] = authorPubkey;
    post["title"] = title;
    post["content"] = content;
    post["timestamp"] = QDateTime::currentSecsSinceEpoch();

    const QByteArray data = QJsonDocument(post).toJson(QJsonDocument::Compact);

    QJsonObject result;

#ifdef LOGOS_CORE_AVAILABLE
    if (m_sync) {
        const QVariant id = m_sync->invokeRemoteMethod("sync_module", "federatedInscribe",
                                                        prefix, QString::fromUtf8(data.toHex()));
        result["success"] = true;
        result["inscriptionId"] = id.toString();
    } else {
        result["success"] = false;
        result["error"] = "sync_module not available";
    }
#else
    m_posts[prefix].append(QString::fromUtf8(data));
    result["success"] = true;
    result["inscriptionId"] = post["id"].toString();
#endif

    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

QString YoloBoard::getPosts(const QString &prefix) {
    QJsonObject result;

#ifdef LOGOS_CORE_AVAILABLE
    if (m_sync) {
        const QVariant raw = m_sync->invokeRemoteMethod("sync_module", "federatedGetHistory",
                                                         prefix, QString(), QString("50"));
        result["success"] = true;
        // Parse inscriptions -> posts
        QJsonArray posts;
        const QJsonDocument doc = QJsonDocument::fromJson(raw.toString().toUtf8());
        if (doc.isObject()) {
            const QJsonArray inscriptions = doc.object()["inscriptions"].toArray();
            for (const QJsonValue &insc : inscriptions) {
                const QByteArray hex = QByteArray::fromHex(
                    insc.toObject()["data"].toString().toLatin1());
                const QJsonDocument pd = QJsonDocument::fromJson(hex);
                if (pd.isObject()) posts.append(pd.object());
            }
        }
        result["posts"] = posts;
    } else {
        result["success"] = false;
        result["posts"] = QJsonArray();
    }
#else
    QJsonArray posts;
    for (const QString &raw : m_posts.value(prefix)) {
        const QJsonDocument pd = QJsonDocument::fromJson(raw.toUtf8());
        if (pd.isObject()) posts.append(pd.object());
    }
    result["success"] = true;
    result["posts"] = posts;
#endif

    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

void YoloBoard::followBoard(const QString &prefix) {
#ifdef LOGOS_CORE_AVAILABLE
    if (m_sync) {
        m_sync->invokeRemoteMethod("sync_module", "federatedFollow", prefix);
    }
#else
    Q_UNUSED(prefix)
#endif
}
