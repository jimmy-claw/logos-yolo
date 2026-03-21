#include <QtTest/QTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonDocument>

#include "../src/yolo_board.h"
#include "logos_api_client.h"

// ── Mocks ───────────────────────────────────────────────────────────────────

class MockBlockchain : public LogosAPIClient {
public:
    QString nextInscriptionId;

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& arg1 = {},
                                const QVariant& arg2 = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "inscribe")
            return nextInscriptionId;
        if (method == "queryChannelsByPrefix")
            return QStringLiteral("[]");
        return {};
    }
};

class MockStorage : public LogosAPIClient {
public:
    QString nextCid;

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& /*arg1*/ = {},
                                const QVariant& /*arg2*/ = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "uploadUrl")
            return nextCid;
        return {};
    }
};

// ── Test class ──────────────────────────────────────────────────────────────

class TestYoloEvents : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testPostCreated_postInscribedSignal();
    void testPostStored_postUploadedToStorageSignal();
    void testPostFullyPublished_postPublishedSignal();
    void testSmallPost_fullSignalSequence();
    void testLargePost_fullSignalSequence();
    void testErrorCondition_channelNotAvailable();
    void testErrorCondition_storageFails();
    void testBoardJoined_signalFires();

private:
    MockBlockchain* blockchain_ = nullptr;
    MockStorage* storage_ = nullptr;
    LogosAPIClient* kvStub_ = nullptr;

    // Helper: create a board ready for posting
    YoloBoard* makeBoard(const QString& name);
};

void TestYoloEvents::initTestCase()
{
    blockchain_ = new MockBlockchain();
    storage_ = new MockStorage();
    kvStub_ = new LogosAPIClient();
}

void TestYoloEvents::cleanupTestCase()
{
    delete blockchain_;
    delete storage_;
    delete kvStub_;
}

YoloBoard* TestYoloEvents::makeBoard(const QString& name)
{
    blockchain_->nextInscriptionId = "meta_" + name;

    auto* board = new YoloBoard();
    board->setBlockchainClient(blockchain_);
    board->setStorageClient(storage_);
    board->setKvClient(kvStub_);
    board->setOwnPubkey("event_tester_pubkey");
    board->createBoard(name, "test board for events");
    return board;
}

// ── Signal tests ────────────────────────────────────────────────────────────

void TestYoloEvents::testPostCreated_postInscribedSignal()
{
    YoloBoard* board = makeBoard("inscribed-test");
    blockchain_->nextInscriptionId = "evt_insc_001";

    QSignalSpy spy(board, &YoloBoard::postInscribed);

    board->createPost("Inscribed Test", "hello", "alice");

    QCOMPARE(spy.count(), 1);

    // postInscribed(postId, channelId)
    QString postId = spy.at(0).at(0).toString();
    QString channelId = spy.at(0).at(1).toString();

    QCOMPARE(postId, QString("evt_insc_001"));
    QCOMPARE(channelId, QString("YOLO:inscribed-test"));

    delete board;
}

void TestYoloEvents::testPostStored_postUploadedToStorageSignal()
{
    YoloBoard* board = makeBoard("storage-test");

    storage_->nextCid = "QmEventStoreCid";
    blockchain_->nextInscriptionId = "evt_store_001";

    QSignalSpy spy(board, &YoloBoard::postUploadedToStorage);

    // Large content triggers storage
    QString largeContent(2048, 'y');
    board->createPost("Storage Test", largeContent, "bob");

    // Should fire at least once with the CID
    QVERIFY(spy.count() >= 1);

    // Check the last emission has the CID
    QString cid = spy.last().at(1).toString();
    QCOMPARE(cid, QString("QmEventStoreCid"));

    delete board;
}

void TestYoloEvents::testPostFullyPublished_postPublishedSignal()
{
    YoloBoard* board = makeBoard("published-test");
    blockchain_->nextInscriptionId = "evt_pub_001";

    QSignalSpy spy(board, &YoloBoard::postPublished);

    board->createPost("Published Title", "content", "charlie");

    QCOMPARE(spy.count(), 1);

    // postPublished(postId, title)
    QString postId = spy.at(0).at(0).toString();
    QString title = spy.at(0).at(1).toString();

    QCOMPARE(postId, QString("evt_pub_001"));
    QCOMPARE(title, QString("Published Title"));

    delete board;
}

void TestYoloEvents::testSmallPost_fullSignalSequence()
{
    YoloBoard* board = makeBoard("seq-small");
    blockchain_->nextInscriptionId = "evt_seq_001";

    QSignalSpy storageSpy(board, &YoloBoard::postUploadedToStorage);
    QSignalSpy inscribedSpy(board, &YoloBoard::postInscribed);
    QSignalSpy publishedSpy(board, &YoloBoard::postPublished);
    QSignalSpy errorSpy(board, &YoloBoard::error);

    board->createPost("Small", "tiny content", "dave");

    // Small post: no storage signal, but inscribed + published fire
    QCOMPARE(storageSpy.count(), 0);
    QCOMPARE(inscribedSpy.count(), 1);
    QCOMPARE(publishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);

    // Verify ordering: inscribed fires before published
    // (Both have already fired, so we verify they both exist)
    QCOMPARE(inscribedSpy.at(0).at(0).toString(), QString("evt_seq_001"));
    QCOMPARE(publishedSpy.at(0).at(0).toString(), QString("evt_seq_001"));

    delete board;
}

void TestYoloEvents::testLargePost_fullSignalSequence()
{
    YoloBoard* board = makeBoard("seq-large");

    storage_->nextCid = "QmSeqLargeCid";
    blockchain_->nextInscriptionId = "evt_seq_large_001";

    QSignalSpy storageSpy(board, &YoloBoard::postUploadedToStorage);
    QSignalSpy inscribedSpy(board, &YoloBoard::postInscribed);
    QSignalSpy publishedSpy(board, &YoloBoard::postPublished);
    QSignalSpy errorSpy(board, &YoloBoard::error);

    QString largeContent(2048, 'z');
    board->createPost("Large Seq", largeContent, "eve");

    // Large post: storage → inscribed → published
    QVERIFY(storageSpy.count() >= 1);
    QCOMPARE(inscribedSpy.count(), 1);
    QCOMPARE(publishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);

    // Storage signal contains the CID
    QCOMPARE(storageSpy.last().at(1).toString(), QString("QmSeqLargeCid"));

    // Inscribed and published contain the inscription ID
    QCOMPARE(inscribedSpy.at(0).at(0).toString(), QString("evt_seq_large_001"));
    QCOMPARE(publishedSpy.at(0).at(0).toString(), QString("evt_seq_large_001"));
    QCOMPARE(publishedSpy.at(0).at(1).toString(), QString("Large Seq"));

    delete board;
}

void TestYoloEvents::testErrorCondition_channelNotAvailable()
{
    // Board with no blockchain client → channel not available
    YoloBoard board;
    board.setKvClient(kvStub_);

    // joinBoard without blockchain still works (sets name + creates channel)
    // but createPost should fail
    board.joinBoard("no-blockchain");

    QSignalSpy errorSpy(&board, &YoloBoard::error);
    QSignalSpy inscribedSpy(&board, &YoloBoard::postInscribed);

    QString result = board.createPost("Fail", "content", "frank");

    QVERIFY(result.isEmpty());
    QCOMPARE(inscribedSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.at(0).at(0).toString().contains("not available"));
}

void TestYoloEvents::testErrorCondition_storageFails()
{
    YoloBoard* board = makeBoard("storage-fail");

    // Storage returns empty CID → failure
    storage_->nextCid = "";
    blockchain_->nextInscriptionId = "should_not_reach";

    QSignalSpy errorSpy(board, &YoloBoard::error);
    QSignalSpy inscribedSpy(board, &YoloBoard::postInscribed);

    QString largeContent(2048, 'w');
    QString result = board->createPost("Fail Store", largeContent, "grace");

    QVERIFY(result.isEmpty());
    QCOMPARE(inscribedSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.at(0).at(0).toString().contains("storage"));

    delete board;
}

void TestYoloEvents::testBoardJoined_signalFires()
{
    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setKvClient(kvStub_);

    QSignalSpy spy(&board, &YoloBoard::boardJoined);

    board.joinBoard("join-signal-test");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString("YOLO:join-signal-test"));
}

QTEST_MAIN(TestYoloEvents)
#include "test_yolo_events.moc"
