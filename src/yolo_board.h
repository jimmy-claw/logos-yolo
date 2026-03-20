#pragma once
#include <QObject>
#include <QString>
#include <QMap>

#ifdef LOGOS_CORE_AVAILABLE
class LogosAPIClient;
#endif

class YoloBoard : public QObject {
    Q_OBJECT
public:
    explicit YoloBoard(QObject *parent = nullptr);

#ifdef LOGOS_CORE_AVAILABLE
    void setSyncClient(LogosAPIClient *client);
#endif

    QString createBoard(const QString &name, const QString &creatorPubkey);
    QString postOpinion(const QString &prefix, const QString &authorPubkey, const QString &title, const QString &content);
    QString getPosts(const QString &prefix);
    void followBoard(const QString &prefix);

private:
    static QString makeBoardPrefix(const QString &creatorPubkey);

#ifdef LOGOS_CORE_AVAILABLE
    LogosAPIClient *m_sync = nullptr;
#else
    QMap<QString, QStringList> m_posts; // prefix -> list of post JSON
#endif
};
