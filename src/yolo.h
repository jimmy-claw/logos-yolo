#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class LogosAPI;
class LogosAPIClient;
class YoloBoard;

class Yolo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentBoard READ currentBoard NOTIFY currentBoardChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool hasBoardSelected READ hasBoardSelected NOTIFY currentBoardChanged)

public:
    explicit Yolo(QObject *parent = nullptr);

    Q_INVOKABLE QString hello() const;
    void initLogos(LogosAPI *logosAPIInstance);

    // Client wiring — called from YoloPlugin::initLogos()
    void setBlockchainClient(LogosAPIClient* client);
    void setKvClient(LogosAPIClient* client);
    void setStorageClient(LogosAPIClient* client);

    void watchBoard(YoloBoard *board);

    QString currentBoard() const;
    bool hasBoardSelected() const;
    QString errorMessage() const;

    Q_INVOKABLE QVariantList discoverBoards();
    Q_INVOKABLE void selectBoard(const QString& boardName);
    Q_INVOKABLE void createNewBoard(const QString& name, const QString& description);
    Q_INVOKABLE QVariantList getPosts(int limit = 50);
    Q_INVOKABLE QString submitPost(const QString& title, const QString& content);
    Q_INVOKABLE void clearError();

signals:
    void eventResponse(const QString &eventName, const QVariantList &args);
    void currentBoardChanged();
    void errorMessageChanged();
    void postsChanged();
    void boardsChanged();
    void newEvent(const QString& eventName, const QString& type, const QString& message);

private:
    void onBoardEvent(const QString& eventName, const QVariantList& args);
    void setError(const QString& msg);
    void ensureBoard();

#ifdef YOLO_HAS_BOARD
    YoloBoard* m_board = nullptr;
    LogosAPIClient* m_blockchain = nullptr;
    LogosAPIClient* m_kv = nullptr;
    LogosAPIClient* m_storage = nullptr;
#endif
    QString m_currentBoard;
    QString m_errorMessage;
};
