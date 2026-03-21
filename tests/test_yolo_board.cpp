#include <QtTest/QTest>
#include <QSignalSpy>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

#include "../src/yolo_board.h"
#include "logos_api_client.h"

// ── Mock LogosAPIClient that returns controllable responses ─────────────────

class MockBlockchainClient : public LogosAPIClient {
public:
    QString nextInscriptionId;
    QString channelInscriptionsJson;
    QList<QPair<QString, QString>> inscribeCalls;  // channelId, dataHex

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& arg1 = {},
                                const QVariant& arg2 = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "inscribe") {
            inscribeCalls.append({arg1.toString(), arg2.toString()});
            return nextInscriptionId;
        }
        if (method == "getChannelInscriptions") {
            return channelInscriptionsJson;
        }
        if (method == "queryChannel") {
            return channelInscriptionsJson;
        }
        if (method == "queryChannelsByPrefix") {
            return QStringLiteral("[]");
        }
        return {};
    }
};

class MockStorageClient : public LogosAPIClient {
public:
    QString nextCid;
    int storeCallCount = 0;

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& /*arg1*/ = {},
                                const QVariant& /*arg2*/ = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "uploadUrl") {
            storeCallCount++;
            return nextCid;
        }
        return {};
    }
};

// ── Test class ──────────────────────────────────────────────────────────────

class TestYoloBoard : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testBoardCreation();
    void testBoardCreation_verifyChannelIdDerivation();
    void testBoardJoin();
    void testBoardPrefix();

    void testPostSerialization();
    void testPostDeserialization();
    void testPostRoundTrip();

    void testCreatePost_jsonSerialization();
    void testCreatePost_signalChain();
    void testCreatePost_largeContent_usesContentStore();

    void testGetPosts_deserializationAndOrdering();
    void testGetPostsEmpty();

    void testLargeContentThreshold();
    void testDiscoverBoards();
    void testUploadDownload();

private:
    MockBlockchainClient* blockchain_ = nullptr;
    MockStorageClient* storage_ = nullptr;
    LogosAPIClient* kvStub_ = nullptr;
};

void TestYoloBoard::initTestCase()
{
    blockchain_ = new MockBlockchainClient();
    storage_ = new MockStorageClient();
    kvStub_ = new LogosAPIClient();
}

void TestYoloBoard::cleanupTestCase()
{
    delete blockchain_;
    delete storage_;
    delete kvStub_;
}

// ── Board management ────────────────────────────────────────────────────────

void TestYoloBoard::testBoardCreation()
{
    blockchain_->nextInscriptionId = "meta_001";

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);
    board.setOwnPubkey("mypubkey123");

    QSignalSpy joinedSpy(&board, &YoloBoard::boardJoined);

    board.createBoard("test-community", "A test community board");

    QCOMPARE(board.boardName(), QString("test-community"));
    QCOMPARE(board.boardPrefix(), QString("YOLO:test-community"));
    QCOMPARE(joinedSpy.count(), 1);
    QCOMPARE(joinedSpy.takeFirst().at(0).toString(), QString("YOLO:test-community"));
}

void TestYoloBoard::testBoardCreation_verifyChannelIdDerivation()
{
    QString prefix = "YOLO:general";
    QString pubkey = "admin_pubkey_abc";
    QString fullId = QString::fromUtf8("λ%1:%2").arg(prefix, pubkey);
    QByteArray hash = QCryptographicHash::hash(fullId.toUtf8(), QCryptographicHash::Sha256);
    QString channelId = hash.toHex();

    QCOMPARE(channelId.length(), 64);

    // Deterministic
    QByteArray hash2 = QCryptographicHash::hash(fullId.toUtf8(), QCryptographicHash::Sha256);
    QCOMPARE(hash.toHex(), hash2.toHex());

    // Different pubkey → different channel
    QString fullId2 = QString::fromUtf8("λ%1:%2").arg(prefix, QString("other_pubkey"));
    QByteArray hash3 = QCryptographicHash::hash(fullId2.toUtf8(), QCryptographicHash::Sha256);
    QVERIFY(hash.toHex() != hash3.toHex());
}

void TestYoloBoard::testBoardJoin()
{
    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);

    QSignalSpy joinedSpy(&board, &YoloBoard::boardJoined);
    board.joinBoard("general");

    QCOMPARE(board.boardName(), QString("general"));
    QCOMPARE(board.boardPrefix(), QString("YOLO:general"));
    QCOMPARE(joinedSpy.count(), 1);
}

void TestYoloBoard::testBoardPrefix()
{
    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.joinBoard("music");
    QCOMPARE(board.boardPrefix(), QString("YOLO:music"));

    YoloBoard board2;
    board2.setBlockchainClient(blockchain_);
    board2.joinBoard("art-gallery");
    QCOMPARE(board2.boardPrefix(), QString("YOLO:art-gallery"));
}

// ── Post serialization ──────────────────────────────────────────────────────

void TestYoloBoard::testPostSerialization()
{
    YoloPost post;
    post.id        = "insc_001";
    post.title     = "Hello World";
    post.content   = "This is a test post.";
    post.author    = "alice";
    post.timestamp = 1700000000000;
    post.cid       = "";

    QJsonObject json = post.toJson();
    QCOMPARE(json["title"].toString(), QString("Hello World"));
    QCOMPARE(json["content"].toString(), QString("This is a test post."));
    QCOMPARE(json["author"].toString(), QString("alice"));
    QCOMPARE(json["timestamp"].toVariant().toLongLong(), qint64(1700000000000));
    QVERIFY(!json.contains("cid"));
}

void TestYoloBoard::testPostDeserialization()
{
    QJsonObject json;
    json["title"]     = "Test Post";
    json["content"]   = "Content here.";
    json["author"]    = "bob";
    json["timestamp"] = qint64(1700000000000);
    json["cid"]       = "QmSomeCid123";

    YoloPost post = YoloPost::fromJson(json, "insc_002");

    QCOMPARE(post.id, QString("insc_002"));
    QCOMPARE(post.title, QString("Test Post"));
    QCOMPARE(post.content, QString("Content here."));
    QCOMPARE(post.author, QString("bob"));
    QCOMPARE(post.timestamp, qint64(1700000000000));
    QCOMPARE(post.cid, QString("QmSomeCid123"));
}

void TestYoloBoard::testPostRoundTrip()
{
    YoloPost original;
    original.id        = "insc_rt";
    original.title     = "Round Trip";
    original.content   = "Foo bar baz";
    original.author    = "charlie";
    original.timestamp = 1700000099999;
    original.cid       = "QmRoundTrip";

    QJsonObject json = original.toJson();
    YoloPost restored = YoloPost::fromJson(json, original.id);

    QCOMPARE(restored.id, original.id);
    QCOMPARE(restored.title, original.title);
    QCOMPARE(restored.content, original.content);
    QCOMPARE(restored.author, original.author);
    QCOMPARE(restored.timestamp, original.timestamp);
    QCOMPARE(restored.cid, original.cid);
}

// ── Post creation with mock ─────────────────────────────────────────────────

void TestYoloBoard::testCreatePost_jsonSerialization()
{
    blockchain_->nextInscriptionId = "post_json_001";
    blockchain_->inscribeCalls.clear();

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);
    board.setOwnPubkey("poster_key");
    board.createBoard("json-test", "testing JSON");

    blockchain_->inscribeCalls.clear();
    blockchain_->nextInscriptionId = "post_json_002";

    QString postId = board.createPost("My Title", "My Content", "alice");
    QCOMPARE(postId, QString("post_json_002"));
    QCOMPARE(blockchain_->inscribeCalls.size(), 1);

    // Decode the inscribed data (hex-encoded on wire)
    QString dataHex = blockchain_->inscribeCalls.first().second;
    QByteArray rawData = QByteArray::fromHex(dataHex.toLatin1());
    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    QVERIFY(!doc.isNull());

    QJsonObject obj = doc.object();
    QCOMPARE(obj["title"].toString(), QString("My Title"));
    QCOMPARE(obj["content"].toString(), QString("My Content"));
    QCOMPARE(obj["author"].toString(), QString("alice"));
    QVERIFY(obj.contains("timestamp"));
    QVERIFY(obj["timestamp"].toVariant().toLongLong() > 0);
}

void TestYoloBoard::testCreatePost_signalChain()
{
    blockchain_->nextInscriptionId = "post_sig_001";

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);
    board.setOwnPubkey("sig_poster");
    board.createBoard("sig-test", "signal test");

    blockchain_->nextInscriptionId = "post_sig_002";

    QSignalSpy inscribedSpy(&board, &YoloBoard::postInscribed);
    QSignalSpy publishedSpy(&board, &YoloBoard::postPublished);
    QSignalSpy storageSpy(&board, &YoloBoard::postUploadedToStorage);

    board.createPost("Signal Post", "Short content", "alice");

    QCOMPARE(storageSpy.count(), 0);

    QCOMPARE(inscribedSpy.count(), 1);
    QCOMPARE(inscribedSpy.at(0).at(0).toString(), QString("post_sig_002"));
    QCOMPARE(inscribedSpy.at(0).at(1).toString(), QString("YOLO:sig-test"));

    QCOMPARE(publishedSpy.count(), 1);
    QCOMPARE(publishedSpy.at(0).at(0).toString(), QString("post_sig_002"));
    QCOMPARE(publishedSpy.at(0).at(1).toString(), QString("Signal Post"));
}

void TestYoloBoard::testCreatePost_largeContent_usesContentStore()
{
    blockchain_->nextInscriptionId = "meta_large";

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setStorageClient(storage_);
    board.setKvClient(kvStub_);
    board.setOwnPubkey("large_poster");
    board.createBoard("large-test", "large content test");

    storage_->nextCid = "QmLargeContentCid123";
    storage_->storeCallCount = 0;
    blockchain_->nextInscriptionId = "post_large_001";
    blockchain_->inscribeCalls.clear();

    QSignalSpy storageSpy(&board, &YoloBoard::postUploadedToStorage);
    QSignalSpy inscribedSpy(&board, &YoloBoard::postInscribed);
    QSignalSpy publishedSpy(&board, &YoloBoard::postPublished);

    QString largeContent(2048, 'x');
    board.createPost("Large Post", largeContent, "bob");

    QCOMPARE(storage_->storeCallCount, 1);
    QVERIFY(storageSpy.count() >= 1);

    auto lastStorageArgs = storageSpy.last();
    QCOMPARE(lastStorageArgs.at(1).toString(), QString("QmLargeContentCid123"));

    QVERIFY(!blockchain_->inscribeCalls.isEmpty());
    QString dataHex = blockchain_->inscribeCalls.last().second;
    QByteArray rawData = QByteArray::fromHex(dataHex.toLatin1());
    QJsonObject obj = QJsonDocument::fromJson(rawData).object();
    QCOMPARE(obj["cid"].toString(), QString("QmLargeContentCid123"));
    QVERIFY(obj["content"].toString().startsWith("cid:"));

    QCOMPARE(inscribedSpy.count(), 1);
    QCOMPARE(publishedSpy.count(), 1);
}

// ── Get posts ───────────────────────────────────────────────────────────────

void TestYoloBoard::testGetPosts_deserializationAndOrdering()
{
    auto makeInscJson = [](const QString& id, const QString& title,
                           const QString& author, qint64 ts, quint64 slot) {
        QJsonObject post;
        post["title"] = title;
        post["content"] = "content for " + title;
        post["author"] = author;
        post["timestamp"] = ts;

        QByteArray postBytes = QJsonDocument(post).toJson(QJsonDocument::Compact);
        QJsonObject insc;
        insc["inscriptionId"] = id;
        insc["channelId"] = "ch_test";
        insc["data"] = QString::fromLatin1(postBytes.toHex());
        insc["slot"] = QJsonValue(static_cast<qint64>(slot));
        return insc;
    };

    QJsonArray inscriptions;
    inscriptions.append(makeInscJson("p1", "First", "alice", 1700000001000, 1));
    inscriptions.append(makeInscJson("p2", "Second", "bob", 1700000002000, 2));
    inscriptions.append(makeInscJson("p3", "Third", "charlie", 1700000003000, 3));

    blockchain_->channelInscriptionsJson =
        QString::fromUtf8(QJsonDocument(inscriptions).toJson(QJsonDocument::Compact));
    blockchain_->nextInscriptionId = "meta_gp";

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);
    board.setOwnPubkey("reader_key");
    board.createBoard("read-test", "reading");

    QList<YoloPost> posts = board.getPosts();

    QCOMPARE(posts.size(), 3);
    QCOMPARE(posts[0].title, QString("Third"));
    QCOMPARE(posts[0].author, QString("charlie"));
    QCOMPARE(posts[0].id, QString("p3"));
    QCOMPARE(posts[1].title, QString("Second"));
    QCOMPARE(posts[2].title, QString("First"));

    blockchain_->channelInscriptionsJson.clear();
}

void TestYoloBoard::testGetPostsEmpty()
{
    blockchain_->channelInscriptionsJson.clear();

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);
    board.joinBoard("empty-board");

    QList<YoloPost> posts = board.getPosts();
    QVERIFY(posts.isEmpty());
}

// ── Miscellaneous ───────────────────────────────────────────────────────────

void TestYoloBoard::testLargeContentThreshold()
{
    QCOMPARE(YoloBoard::CONTENT_STORE_THRESHOLD, 1024);

    QString smallContent(512, 'x');
    QVERIFY(smallContent.toUtf8().size() <= YoloBoard::CONTENT_STORE_THRESHOLD);

    QString largeContent(2048, 'x');
    QVERIFY(largeContent.toUtf8().size() > YoloBoard::CONTENT_STORE_THRESHOLD);
}

void TestYoloBoard::testDiscoverBoards()
{
    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);

    QString result = board.discoverBoards();
    QCOMPARE(result, QString("[]"));
}

void TestYoloBoard::testUploadDownload()
{
    YoloBoard board;
    board.setStorageClient(storage_);

    storage_->nextCid = "QmUploadTest";
    QString cid = board.uploadContent("Hello World");
    QCOMPARE(cid, QString("QmUploadTest"));

    QByteArray data = board.downloadContent("QmFakeCid");
    QVERIFY(data.isEmpty());
}

QTEST_MAIN(TestYoloBoard)
#include "test_yolo_board.moc"
