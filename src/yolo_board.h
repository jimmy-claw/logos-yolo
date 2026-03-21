#include <QObject>
class LogosAPI;
#pragma once


#include <QString>
#include <QList>
#include <QByteArray>
#include <QJsonObject>
#include <QDateTime>
#include <QVariantList>

class LogosAPIClient;

// Forward declarations — logos-pipe types
class FederatedChannel;
class ContentStore;
class ChannelIndexer;

// Deserialized post from a YOLO board.
struct YoloPost {
    QString id;           // inscription ID
    QString title;
    QString content;      // inline content or CID reference
    QString author;
    qint64  timestamp;
    QString cid;          // non-empty if content stored in ContentStore
    QString channelId;

    QJsonObject toJson() const;
    static YoloPost fromJson(const QJsonObject& obj, const QString& postId = {});
};

// YoloBoard — community board backed by a FederatedChannel.
//
// Each board maps to a FederatedChannel with prefix "YOLO:<boardName>".
// Posts are JSON blobs inscribed on-chain. Large content (>1KB) is offloaded
// to ContentStore with the CID referenced in the post JSON.
//
// Post creation pipeline:
//   1. Serialize post to JSON
//   2. If content > 1KB → store in ContentStore, replace with CID
//   3. Inscribe JSON on FederatedChannel
//   4. Signals fired at each stage
//
// Board discovery uses ChannelIndexer prefix scan for "YOLO".
class YoloBoard : public QObject {
    Q_OBJECT
public:
    explicit YoloBoard(QObject* parent = nullptr);

    // === Client Wiring ===
    void initLogos(LogosAPI* api);
    void setBlockchainClient(LogosAPIClient* blockchain);
    void setStorageClient(LogosAPIClient* storage);
    void setKvClient(LogosAPIClient* kv);
    void setOwnPubkey(const QString& pubkeyHex);

    // === Board Management ===

    // Create/join a named board. Prefix will be "YOLO:<name>".
    void createBoard(const QString& name, const QString& description);
    void joinBoard(const QString& boardId);

    QString boardName() const;
    QString boardPrefix() const;

    // === Posting ===

    // Create a post on the current board.
    // Returns the post ID (inscription ID) on success, empty on failure.
    // Fires: postUploadedToStorage (if large) → postInscribed → postPublished
    QString createPost(const QString& title, const QString& content,
                       const QString& author);

    // Get recent posts from the board, newest first.
    QList<YoloPost> getPosts(int limit = 50);

    // === Storage Integration ===

    // Upload raw content to ContentStore. Returns CID.
    QString uploadContent(const QByteArray& data);

    // Download content by CID from ContentStore.
    QByteArray downloadContent(const QString& cid);

    // === Discovery ===

    // Discover all YOLO boards via ChannelIndexer prefix scan.
    // Returns JSON array: [{"channelId":"...", "inscriptionCount":N}, ...]
    QString discoverBoards();

    // Size threshold for offloading content to ContentStore.
    static constexpr int CONTENT_STORE_THRESHOLD = 1024;  // 1KB

signals:
    void postInscribed(const QString& postId, const QString& channelId);
    void postPublished(const QString& postId, const QString& title);
    void postUploadedToStorage(const QString& postId, const QString& cid);
    void boardJoined(const QString& boardId);
    void error(const QString& message);

    // Structured lifecycle event for UI/CLI consumption.
    // eventName: "post.created", "post.uploading", "post.uploaded",
    //            "post.inscribing", "post.inscribed", "post.published"
    // args: [type (info/success/error), message]
    void eventResponse(const QString& eventName, const QVariantList& args);

private:
    void ensureFederatedChannel();
    void ensureContentStore();
    void ensureIndexer();

    QByteArray serializePost(const QString& title, const QString& content,
                             const QString& author) const;

    static QString boardPrefix(const QString& name);

    QString m_boardName;
    QString m_ownPubkey;

    LogosAPIClient* m_blockchain = nullptr;
    LogosAPIClient* m_storage = nullptr;
    LogosAPIClient* m_kv = nullptr;

    FederatedChannel* m_channel = nullptr;
    ContentStore* m_contentStore = nullptr;
    ChannelIndexer* m_indexer = nullptr;
};
