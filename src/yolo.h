#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class LogosAPI;  // Forward declaration
class YoloBoard; // Forward declaration

/**
 * Yolo — YOLO community board logic for Logos UI plugin.
 *
 * Owns YoloBoard instances and forwards their eventResponse signals
 * so that YoloPlugin can route them to Logos Core for UI/CLI display.
 *
 * Exposes board operations to QML: discovery, selection, posting, events.
 */
class Yolo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentBoard READ currentBoard NOTIFY currentBoardChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool hasBoardSelected READ hasBoardSelected NOTIFY currentBoardChanged)

public:
    explicit Yolo(QObject *parent = nullptr);

    Q_INVOKABLE QString hello() const;
    void initLogos(LogosAPI *logosAPIInstance);

    // Connect an external YoloBoard's eventResponse to this module's eventResponse.
    void watchBoard(YoloBoard *board);

    // Properties
    QString currentBoard() const;
    bool hasBoardSelected() const;
    QString errorMessage() const;

    // QML-invokable board operations
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
#endif
    QString m_currentBoard;
    QString m_errorMessage;
};
