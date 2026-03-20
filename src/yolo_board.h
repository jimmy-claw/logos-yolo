#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

class LogosAPIClient;

class YoloBoard : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentPrefix READ currentPrefix NOTIFY boardCreated)
    Q_PROPERTY(QStringList posts READ getPostsList NOTIFY postsLoaded)

public:
    explicit YoloBoard(QObject *parent = nullptr);
    ~YoloBoard() = default;

    void initWithClient(LogosAPIClient* client);  // Direct client injection

    Q_INVOKABLE QString createBoard(const QString &name, const QString &creatorPubkey);
    Q_INVOKABLE QString postOpinion(const QString &prefix, const QString &authorPubkey,
                                   const QString &title, const QString &content);
    Q_INVOKABLE QString getPosts(const QString &prefix);
    Q_INVOKABLE void followBoard(const QString &prefix);

    QString currentPrefix() const { return m_currentPrefix; }
    QStringList getPostsList() const { return m_posts; }

signals:
    void boardCreated(const QString &prefix);
    void postsLoaded();
    void error(const QString &msg);

private:
    static QString makeBoardPrefix(const QString &creatorPubkey);

    QString m_currentPrefix;
    QStringList m_posts;
    QMap<QString, QStringList> m_allPosts;
    LogosAPIClient* m_client = nullptr;  // Direct to sync_module
};
