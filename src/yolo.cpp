#include "yolo.h"

#ifdef YOLO_HAS_BOARD
#include "yolo_board.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#endif

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
#ifdef YOLO_HAS_BOARD
    ensureBoard();
    if (m_board && logosAPIInstance)
        m_board->initLogos(logosAPIInstance);
#else
    Q_UNUSED(logosAPIInstance)
#endif
}

void Yolo::watchBoard(YoloBoard *board) {
#ifdef YOLO_HAS_BOARD
    if (!board) return;
    connect(board, &YoloBoard::eventResponse,
            this, &Yolo::eventResponse);
#else
    Q_UNUSED(board)
#endif
}

// ── Properties ─────────────────────────────────────────────────────────────

QString Yolo::currentBoard() const {
    return m_currentBoard;
}

bool Yolo::hasBoardSelected() const {
    return !m_currentBoard.isEmpty();
}

QString Yolo::errorMessage() const {
    return m_errorMessage;
}

void Yolo::setError(const QString& msg) {
    if (m_errorMessage != msg) {
        m_errorMessage = msg;
        emit errorMessageChanged();
    }
}

void Yolo::clearError() {
    setError(QString());
}

// ── Internal helpers ───────────────────────────────────────────────────────

void Yolo::ensureBoard() {
#ifdef YOLO_HAS_BOARD
    if (!m_board) {
        m_board = new YoloBoard(this);
        connect(m_board, &YoloBoard::eventResponse,
                this, &Yolo::onBoardEvent);
        connect(m_board, &YoloBoard::eventResponse,
                this, &Yolo::eventResponse);
        connect(m_board, &YoloBoard::error,
                this, &Yolo::setError);
        // Wire up already-set clients
        if (m_blockchain) m_board->setBlockchainClient(m_blockchain);
        if (m_kv)         m_board->setKvClient(m_kv);
        if (m_storage)    m_board->setStorageClient(m_storage);
    }
#endif
}

void Yolo::onBoardEvent(const QString& eventName, const QVariantList& args) {
    QString type = args.size() > 0 ? args[0].toString() : "info";
    QString message = args.size() > 1 ? args[1].toString() : eventName;
    emit newEvent(eventName, type, message);

    if (eventName == "post.published") {
        emit postsChanged();
    }
}

// ── QML-invokable board operations ─────────────────────────────────────────

QVariantList Yolo::discoverBoards() {
#ifdef YOLO_HAS_BOARD
    ensureBoard();
    QString json = m_board->discoverBoards();
    if (json.isEmpty()) return {};

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isArray()) return {};

    QVariantList result;
    for (const QJsonValue& val : doc.array()) {
        QJsonObject obj = val.toObject();
        QVariantMap board;
        board["channelId"] = obj["channelId"].toString();
        board["inscriptionCount"] = obj["inscriptionCount"].toInt();
        // Extract board name from channelId (strip "YOLO:" prefix)
        QString chId = obj["channelId"].toString();
        board["name"] = chId.startsWith("YOLO:") ? chId.mid(5) : chId;
        result.append(board);
    }
    return result;
#else
    setError("Board support not available in this build");
    return {};
#endif
}

void Yolo::selectBoard(const QString& boardName) {
#ifdef YOLO_HAS_BOARD
    // Create a fresh YoloBoard for the new board
    if (m_board) {
        delete m_board;
        m_board = nullptr;
    }
    ensureBoard();
    m_board->joinBoard(boardName);
    m_currentBoard = boardName;
    emit currentBoardChanged();
    emit postsChanged();
    clearError();
#else
    Q_UNUSED(boardName)
    setError("Board support not available in this build");
#endif
}

void Yolo::createNewBoard(const QString& name, const QString& description) {
#ifdef YOLO_HAS_BOARD
    // Create a fresh YoloBoard for the new board
    if (m_board) {
        delete m_board;
        m_board = nullptr;
    }
    ensureBoard();
    m_board->createBoard(name, description);
    m_currentBoard = name;
    emit currentBoardChanged();
    emit boardsChanged();
    clearError();
#else
    Q_UNUSED(name)
    Q_UNUSED(description)
    setError("Board support not available in this build");
#endif
}

QVariantList Yolo::getPosts(int limit) {
#ifdef YOLO_HAS_BOARD
    if (m_currentBoard.isEmpty()) return {};
    ensureBoard();

    QList<YoloPost> posts = m_board->getPosts(limit);
    QVariantList result;
    for (const YoloPost& post : posts) {
        QVariantMap map;
        map["id"] = post.id;
        map["title"] = post.title;
        map["content"] = post.content;
        map["author"] = post.author;
        map["timestamp"] = post.timestamp;
        map["cid"] = post.cid;
        map["channelId"] = post.channelId;
        result.append(map);
    }
    return result;
#else
    Q_UNUSED(limit)
    return {};
#endif
}

QString Yolo::submitPost(const QString& title, const QString& content) {
#ifdef YOLO_HAS_BOARD
    if (m_currentBoard.isEmpty()) {
        setError("No board selected");
        return {};
    }
    ensureBoard();
    clearError();
    QString id = m_board->createPost(title, content, "anonymous");
    if (id.isEmpty()) {
        // Error already set by YoloBoard::error signal
    }
    return id;
#else
    Q_UNUSED(title)
    Q_UNUSED(content)
    setError("Board support not available in this build");
    return {};
#endif
}

// ── Client wiring ───────────────────────────────────────────────────────────

void Yolo::setBlockchainClient(LogosAPIClient* client) {
#ifdef YOLO_HAS_BOARD
    m_blockchain = client;
    if (m_board) m_board->setBlockchainClient(client);
#else
    Q_UNUSED(client)
#endif
}

void Yolo::setKvClient(LogosAPIClient* client) {
#ifdef YOLO_HAS_BOARD
    m_kv = client;
    if (m_board) m_board->setKvClient(client);
#else
    Q_UNUSED(client)
#endif
}

void Yolo::setStorageClient(LogosAPIClient* client) {
#ifdef YOLO_HAS_BOARD
    m_storage = client;
    if (m_board) m_board->setStorageClient(client);
#else
    Q_UNUSED(client)
#endif
}
