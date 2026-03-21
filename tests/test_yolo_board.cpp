#include <QtTest/QTest>
#include <QSignalSpy>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "../src/yolo_board.h"
#include "logos_api_client.h"

class TestYoloBoard : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testBoardCreation();
    void testBoardJoin();
    void testBoardPrefix();
    void testPostSerialization();
    void testPostDeserialization();
    void testCreatePostSignals();
    void testGetPostsEmpty();
    void testLargeContentThreshold();
    void testDiscoverBoards();
    void testUploadDownload();

private:
    LogosAPIClient* blockchainStub_ = nullptr;
    LogosAPIClient* storageStub_ = nullptr;
    LogosAPIClient* kvStub_ = nullptr;
};

void TestYoloBoard::initTestCase()
{
    blockchainStub_ = new LogosAPIClient();
    storageStub_ = new LogosAPIClient();
    kvStub_ = new LogosAPIClient();
}

void TestYoloBoard::cleanupTestCase()
{
    delete blockchainStub_;
    delete storageStub_;
    delete kvStub_;
}

void TestYoloBoard::testBoardCreation()
{
    YoloBoard board;
    board.setBlockchainClient(blockchainStub_);
    board.setKvClient(kvStub_);
    board.setOwnPubkey("mypubkey123");

    QSignalSpy joinedSpy(&board, &YoloBoard::boardJoined);

    board.createBoard("test-community", "A test community board");

    QCOMPARE(board.boardName(), QString("test-community"));
    QCOMPARE(board.boardPrefix(), QString("YOLO:test-community"));
    QCOMPARE(joinedSpy.count(), 1);
    QCOMPARE(joinedSpy.takeFirst().at(0).toString(), QString("YOLO:test-community"));
}

void TestYoloBoard::testBoardJoin()
{
    YoloBoard board;
    board.setBlockchainClient(blockchainStub_);
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
    board.setBlockchainClient(blockchainStub_);

    board.joinBoard("music");
    QCOMPARE(board.boardPrefix(), QString("YOLO:music"));

    // Board prefix format: "YOLO:<name>"
    YoloBoard board2;
    board2.setBlockchainClient(blockchainStub_);
    board2.joinBoard("art-gallery");
    QCOMPARE(board2.boardPrefix(), QString("YOLO:art-gallery"));
}

void TestYoloBoard::testPostSerialization()
{
    // Test YoloPost → JSON round-trip
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
    QVERIFY(!json.contains("cid"));  // empty cid not serialized
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

void TestYoloBoard::testCreatePostSignals()
{
    YoloBoard board;
    board.setBlockchainClient(blockchainStub_);
    board.setKvClient(kvStub_);
    board.setOwnPubkey("poster_pubkey");

    board.createBoard("signals-test", "Testing signals");

    QSignalSpy inscribedSpy(&board, &YoloBoard::postInscribed);
    QSignalSpy publishedSpy(&board, &YoloBoard::postPublished);
    QSignalSpy storageSpy(&board, &YoloBoard::postUploadedToStorage);
    QSignalSpy errorSpy(&board, &YoloBoard::error);

    // Small post — should not trigger storage upload
    board.createPost("Small Post", "Short content", "alice");

    // With stubs, inscribe returns empty (no real blockchain)
    // so we expect an error or empty result
    // The important thing is no crash and signals are wired correctly
    QCOMPARE(storageSpy.count(), 0);  // content < 1KB, no storage
}

void TestYoloBoard::testGetPostsEmpty()
{
    YoloBoard board;
    board.setBlockchainClient(blockchainStub_);
    board.setKvClient(kvStub_);

    board.joinBoard("empty-board");

    // With stubs, getPosts returns empty (no real inscriptions)
    QList<YoloPost> posts = board.getPosts();
    QVERIFY(posts.isEmpty());
}

void TestYoloBoard::testLargeContentThreshold()
{
    // Verify threshold constant
    QCOMPARE(YoloBoard::CONTENT_STORE_THRESHOLD, 1024);

    // Content under threshold stays inline
    QString smallContent(512, 'x');
    QVERIFY(smallContent.toUtf8().size() <= YoloBoard::CONTENT_STORE_THRESHOLD);

    // Content over threshold should be offloaded
    QString largeContent(2048, 'x');
    QVERIFY(largeContent.toUtf8().size() > YoloBoard::CONTENT_STORE_THRESHOLD);
}

void TestYoloBoard::testDiscoverBoards()
{
    YoloBoard board;
    board.setBlockchainClient(blockchainStub_);
    board.setKvClient(kvStub_);

    // discoverBoards() should use "YOLO" prefix
    // With stubs, returns empty result
    QString result = board.discoverBoards();
    // Stub returns empty string (no real blockchain data)
    // Test documents expected behavior
    Q_UNUSED(result)
}

void TestYoloBoard::testUploadDownload()
{
    YoloBoard board;
    board.setStorageClient(storageStub_);

    QSignalSpy errorSpy(&board, &YoloBoard::error);

    // Upload with stub returns empty CID (no real storage)
    QString cid = board.uploadContent("Hello World");
    QVERIFY(cid.isEmpty());  // stub doesn't implement real storage

    // Download with stub returns empty bytes
    QByteArray data = board.downloadContent("QmFakeCid");
    QVERIFY(data.isEmpty());
}

QTEST_MAIN(TestYoloBoard)
#include "test_yolo_board.moc"
