#include "yolo_board.h"

#include <federated_channel.h>
#include <content_store.h>
#include <channel_indexer.h>
#include <sync_types.h>

#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <algorithm>

// ── YoloPost ────────────────────────────────────────────────────────────────

QJsonObject YoloPost::toJson() const
{
    QJsonObject obj;
    obj["title"]     = title;
    obj["content"]   = content;
    obj["author"]    = author;
    obj["timestamp"] = timestamp;
    if (!cid.isEmpty()) {
        obj["cid"] = cid;
    }
    return obj;
}

YoloPost YoloPost::fromJson(const QJsonObject& obj, const QString& postId)
{
    YoloPost post;
    post.id        = postId;
    post.title     = obj["title"].toString();
    post.content   = obj["content"].toString();
    post.author    = obj["author"].toString();
    post.timestamp = obj["timestamp"].toVariant().toLongLong();
    post.cid       = obj["cid"].toString();
    return post;
}

// ── YoloBoard ───────────────────────────────────────────────────────────────

YoloBoard::YoloBoard(QObject* parent)
    : QObject(parent)
{
}

// --- Client wiring ---

void YoloBoard::setBlockchainClient(LogosAPIClient* blockchain)
{
    m_blockchain = blockchain;
    if (m_channel) {
        m_channel->setBlockchainClient(blockchain);
    }
}

void YoloBoard::setStorageClient(LogosAPIClient* storage)
{
    m_storage = storage;
    if (m_contentStore) {
        m_contentStore->setStorageClient(storage);
    }
}

void YoloBoard::setKvClient(LogosAPIClient* kv)
{
    m_kv = kv;
    if (m_channel) {
        m_channel->setKvClient(kv);
    }
}

void YoloBoard::setOwnPubkey(const QString& pubkeyHex)
{
    m_ownPubkey = pubkeyHex;
    if (m_channel) {
        m_channel->setOwnPubkey(pubkeyHex);
    }
}

// --- Board management ---

QString YoloBoard::boardPrefix(const QString& name)
{
    return QStringLiteral("YOLO:%1").arg(name);
}

QString YoloBoard::boardName() const
{
    return m_boardName;
}

QString YoloBoard::boardPrefix() const
{
    return boardPrefix(m_boardName);
}

void YoloBoard::createBoard(const QString& name, const QString& description)
{
    m_boardName = name;
    ensureFederatedChannel();

    // Add ourselves as first admin so we can inscribe
    if (!m_ownPubkey.isEmpty()) {
        m_channel->addAdmin(m_ownPubkey);
    }

    // Inscribe board metadata as first post
    QJsonObject meta;
    meta["type"]        = QStringLiteral("board_meta");
    meta["name"]        = name;
    meta["description"] = description;
    meta["timestamp"]   = QDateTime::currentMSecsSinceEpoch();

    QByteArray payload = QJsonDocument(meta).toJson(QJsonDocument::Compact);
    m_channel->inscribe(payload);

    m_channel->follow();
    emit boardJoined(boardPrefix());

    qDebug() << "YoloBoard: created board" << name;
}

void YoloBoard::joinBoard(const QString& boardId)
{
    // boardId is the board name (prefix portion after "YOLO:")
    m_boardName = boardId;
    ensureFederatedChannel();

    m_channel->follow();
    emit boardJoined(boardPrefix());

    qDebug() << "YoloBoard: joined board" << boardId;
}

// --- Posting ---

QByteArray YoloBoard::serializePost(const QString& title, const QString& content,
                                    const QString& author) const
{
    QJsonObject obj;
    obj["title"]     = title;
    obj["content"]   = content;
    obj["author"]    = author;
    obj["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QString YoloBoard::createPost(const QString& title, const QString& content,
                              const QString& author)
{
    ensureFederatedChannel();

    if (!m_channel->isAvailable()) {
        emit error("Board channel not available");
        return {};
    }

    QString contentForPost = content;
    QString storedCid;

    // Step 1: Offload large content to ContentStore
    if (content.toUtf8().size() > CONTENT_STORE_THRESHOLD) {
        ensureContentStore();
        if (m_contentStore && m_contentStore->isAvailable()) {
            storedCid = m_contentStore->store(content.toUtf8());
            if (storedCid.isEmpty()) {
                emit error("Failed to upload content to storage");
                return {};
            }
            // Replace content with CID reference
            contentForPost = QStringLiteral("cid:%1").arg(storedCid);
            emit postUploadedToStorage(QString(), storedCid);
        }
    }

    // Step 2: Serialize post JSON
    QByteArray postJson = serializePost(title, contentForPost, author);
    if (!storedCid.isEmpty()) {
        // Embed CID in the post JSON
        QJsonObject obj = QJsonDocument::fromJson(postJson).object();
        obj["cid"] = storedCid;
        postJson = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    }

    // Step 3: Inscribe on FederatedChannel
    QString inscriptionId = m_channel->inscribe(postJson);
    if (inscriptionId.isEmpty()) {
        emit error("Failed to inscribe post on channel");
        return {};
    }

    // Update storage signal with actual post ID
    if (!storedCid.isEmpty()) {
        emit postUploadedToStorage(inscriptionId, storedCid);
    }

    emit postInscribed(inscriptionId, boardPrefix());
    emit postPublished(inscriptionId, title);

    qDebug() << "YoloBoard: post created" << inscriptionId << title;
    return inscriptionId;
}

QList<YoloPost> YoloBoard::getPosts(int limit)
{
    ensureFederatedChannel();

    if (!m_channel->isAvailable()) {
        emit error("Board channel not available");
        return {};
    }

    IndexerPage page = m_channel->history();
    QList<YoloPost> posts;

    for (const Inscription& insc : page.inscriptions) {
        QJsonDocument doc = QJsonDocument::fromJson(insc.data);
        if (doc.isNull()) continue;

        QJsonObject obj = doc.object();

        // Skip board metadata entries
        if (obj["type"].toString() == "board_meta") continue;

        YoloPost post = YoloPost::fromJson(obj, insc.inscriptionId);
        post.channelId = insc.channelId;
        posts.append(post);

        if (posts.size() >= limit) break;
    }

    // Newest first
    std::reverse(posts.begin(), posts.end());
    return posts;
}

// --- Storage integration ---

QString YoloBoard::uploadContent(const QByteArray& data)
{
    ensureContentStore();
    if (!m_contentStore || !m_contentStore->isAvailable()) {
        emit error("Content store not available");
        return {};
    }
    return m_contentStore->store(data);
}

QByteArray YoloBoard::downloadContent(const QString& cid)
{
    ensureContentStore();
    if (!m_contentStore || !m_contentStore->isAvailable()) {
        emit error("Content store not available");
        return {};
    }
    return m_contentStore->fetch(cid);
}

// --- Discovery ---

QString YoloBoard::discoverBoards()
{
    ensureIndexer();
    if (!m_indexer) {
        emit error("Indexer not available");
        return {};
    }
    return m_indexer->discoverChannels("YOLO");
}

// --- Private helpers ---

void YoloBoard::ensureFederatedChannel()
{
    if (m_channel) return;

    m_channel = new FederatedChannel(boardPrefix(), this);
    if (m_blockchain) m_channel->setBlockchainClient(m_blockchain);
    if (m_kv)         m_channel->setKvClient(m_kv);
    if (!m_ownPubkey.isEmpty()) m_channel->setOwnPubkey(m_ownPubkey);
}

void YoloBoard::ensureContentStore()
{
    if (m_contentStore) return;

    m_contentStore = new ContentStore(this);
    if (m_storage) m_contentStore->setStorageClient(m_storage);
}

void YoloBoard::ensureIndexer()
{
    if (m_indexer) return;

    m_indexer = new ChannelIndexer(this);
    if (m_blockchain) m_indexer->setBlockchainClient(m_blockchain);
    if (m_kv)         m_indexer->setKvClient(m_kv);
}
