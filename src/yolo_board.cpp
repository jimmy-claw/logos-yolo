#include "yolo_board.h"
#include <logos_api_client.h>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>

YoloBoard::YoloBoard(QObject *parent) : QObject(parent) {}

void YoloBoard::initWithClient(LogosAPIClient* client) {
    m_client = client;
    qDebug() << "YoloBoard: sync_module client:" << (client ? "connected" : "null");
}

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
    m_currentPrefix = prefix;

    if (m_client) {
        m_client->invokeRemoteMethod("sync_module", "federatedAddAdmin", prefix, creatorPubkey);
        qDebug() << "YoloBoard: registered board on sync_module:" << prefix;
    }
    emit boardCreated(prefix);
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
    if (m_client) {
        const QVariant id = m_client->invokeRemoteMethod("sync_module", "federatedInscribe",
                                                        prefix, QString::fromUtf8(data.toHex()));
        result["success"] = true;
        result["inscriptionId"] = id.toString();
        qDebug() << "YoloBoard: posted, id:" << id.toString();
    } else {
        m_allPosts[prefix].append(QString::fromUtf8(data));
        result["success"] = true;
        result["inscriptionId"] = post["id"].toString();
        result["stub"] = true;
    }
    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

QString YoloBoard::getPosts(const QString &prefix) {
    QJsonObject result;
    if (m_client) {
        const QVariant raw = m_client->invokeRemoteMethod("sync_module", "federatedGetHistory",
                                                         prefix, QString(), QString("50"));
        QJsonArray posts;
        const QJsonDocument doc = QJsonDocument::fromJson(raw.toString().toUtf8());
        if (doc.isObject()) {
            for (const QJsonValue &insc : doc.object()["inscriptions"].toArray()) {
                const QByteArray hex = QByteArray::fromHex(
                    insc.toObject()["data"].toString().toLatin1());
                const QJsonDocument pd = QJsonDocument::fromJson(hex);
                if (pd.isObject()) posts.append(pd.object());
            }
        }
        result["success"] = true;
        result["posts"] = posts;
    } else {
        QJsonArray posts;
        for (const QString &raw : m_allPosts.value(prefix)) {
            const QJsonDocument pd = QJsonDocument::fromJson(raw.toUtf8());
            if (pd.isObject()) posts.append(pd.object());
        }
        result["success"] = true;
        result["posts"] = posts;
    }
    emit postsLoaded();
    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

void YoloBoard::followBoard(const QString &prefix) {
    if (m_client) {
        m_client->invokeRemoteMethod("sync_module", "federatedFollow", prefix);
    }
}
