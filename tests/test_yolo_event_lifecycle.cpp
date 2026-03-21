#include <QtTest/QTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonDocument>

#include "../src/yolo_board.h"
#include "../src/yolo.h"
#include "logos_api_client.h"

// ── Mocks ───────────────────────────────────────────────────────────────────

class LifecycleMockBlockchain : public LogosAPIClient {
public:
    QString nextInscriptionId;

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& /*arg1*/ = {},
                                const QVariant& /*arg2*/ = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "inscribe")
            return nextInscriptionId;
        if (method == "queryChannelsByPrefix")
            return QStringLiteral("[]");
        return {};
    }
};

class LifecycleMockStorage : public LogosAPIClient {
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

class TestYoloEventLifecycle : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Core lifecycle tests
    void testSmallPost_sixEventSequence();
    void testLargePost_sixEventSequence();
    void testEventsContainRealData();
    void testEventTypes_infoAndSuccess();

    // Plugin wiring test
    void testYoloModuleForwardsEvents();
    void testYoloPluginForwardsEvents();

    // Error event test
    void testErrorCondition_noEventAfterFailure();

private:
    LifecycleMockBlockchain* blockchain_ = nullptr;
    LifecycleMockStorage* storage_ = nullptr;
    LogosAPIClient* kvStub_ = nullptr;

    YoloBoard* makeBoard(const QString& name);
};

void TestYoloEventLifecycle::initTestCase()
{
    blockchain_ = new LifecycleMockBlockchain();
    storage_ = new LifecycleMockStorage();
    kvStub_ = new LogosAPIClient();
}

void TestYoloEventLifecycle::cleanupTestCase()
{
    delete blockchain_;
    delete storage_;
    delete kvStub_;
}

YoloBoard* TestYoloEventLifecycle::makeBoard(const QString& name)
{
    blockchain_->nextInscriptionId = "meta_" + name;

    auto* board = new YoloBoard();
    board->setBlockchainClient(blockchain_);
    board->setStorageClient(storage_);
    board->setKvClient(kvStub_);
    board->setOwnPubkey("lifecycle_test_pubkey");
    board->createBoard(name, "lifecycle test board");
    return board;
}

// ── Small post: 4 events (no storage) ───────────────────────────────────────

void TestYoloEventLifecycle::testSmallPost_sixEventSequence()
{
    YoloBoard* board = makeBoard("small-lifecycle");
    blockchain_->nextInscriptionId = "lc_small_001";

    QSignalSpy eventSpy(board, &YoloBoard::eventResponse);

    board->createPost("Hello World", "short content", "alice");

    // Small post: post.created → post.inscribing → post.inscribed → post.published
    QCOMPARE(eventSpy.count(), 4);

    QCOMPARE(eventSpy.at(0).at(0).toString(), QString("post.created"));
    QCOMPARE(eventSpy.at(1).at(0).toString(), QString("post.inscribing"));
    QCOMPARE(eventSpy.at(2).at(0).toString(), QString("post.inscribed"));
    QCOMPARE(eventSpy.at(3).at(0).toString(), QString("post.published"));

    delete board;
}

// ── Large post: all 6 events ────────────────────────────────────────────────

void TestYoloEventLifecycle::testLargePost_sixEventSequence()
{
    YoloBoard* board = makeBoard("large-lifecycle");
    storage_->nextCid = "QmLifecycleCid123";
    blockchain_->nextInscriptionId = "lc_large_001";

    QSignalSpy eventSpy(board, &YoloBoard::eventResponse);

    QString largeContent(2048, 'x');
    board->createPost("Big Post", largeContent, "bob");

    // Large post: all 6 events fire
    QCOMPARE(eventSpy.count(), 6);

    QCOMPARE(eventSpy.at(0).at(0).toString(), QString("post.created"));
    QCOMPARE(eventSpy.at(1).at(0).toString(), QString("post.uploading"));
    QCOMPARE(eventSpy.at(2).at(0).toString(), QString("post.uploaded"));
    QCOMPARE(eventSpy.at(3).at(0).toString(), QString("post.inscribing"));
    QCOMPARE(eventSpy.at(4).at(0).toString(), QString("post.inscribed"));
    QCOMPARE(eventSpy.at(5).at(0).toString(), QString("post.published"));

    delete board;
}

// ── Events contain real CIDs, channelIds, inscriptionIds ────────────────────

void TestYoloEventLifecycle::testEventsContainRealData()
{
    YoloBoard* board = makeBoard("data-check");
    storage_->nextCid = "QmRealCid456";
    blockchain_->nextInscriptionId = "insc_real_789";

    QSignalSpy eventSpy(board, &YoloBoard::eventResponse);

    QString largeContent(2048, 'y');
    board->createPost("Data Post", largeContent, "charlie");

    QCOMPARE(eventSpy.count(), 6);

    // Event 1: post.created contains title
    QVariantList createdArgs = eventSpy.at(0).at(1).toList();
    QVERIFY(createdArgs.at(1).toString().contains("Data Post"));

    // Event 3: post.uploaded contains the real CID
    QVariantList uploadedArgs = eventSpy.at(2).at(1).toList();
    QVERIFY(uploadedArgs.at(1).toString().contains("QmRealCid456"));

    // Event 5: post.inscribed contains real channelId and inscriptionId
    QVariantList inscribedArgs = eventSpy.at(4).at(1).toList();
    QString inscribedMsg = inscribedArgs.at(1).toString();
    QVERIFY(inscribedMsg.contains("YOLO:data-check"));
    QVERIFY(inscribedMsg.contains("insc_real_789"));

    delete board;
}

// ── Event types are correctly set ───────────────────────────────────────────

void TestYoloEventLifecycle::testEventTypes_infoAndSuccess()
{
    YoloBoard* board = makeBoard("types-check");
    storage_->nextCid = "QmTypesCid";
    blockchain_->nextInscriptionId = "insc_types";

    QSignalSpy eventSpy(board, &YoloBoard::eventResponse);

    QString largeContent(2048, 'z');
    board->createPost("Types Post", largeContent, "dave");

    QCOMPARE(eventSpy.count(), 6);

    // Info events: created, uploading, inscribing
    QCOMPARE(eventSpy.at(0).at(1).toList().at(0).toString(), QString("info"));
    QCOMPARE(eventSpy.at(1).at(1).toList().at(0).toString(), QString("info"));
    QCOMPARE(eventSpy.at(3).at(1).toList().at(0).toString(), QString("info"));

    // Success events: uploaded, inscribed, published
    QCOMPARE(eventSpy.at(2).at(1).toList().at(0).toString(), QString("success"));
    QCOMPARE(eventSpy.at(4).at(1).toList().at(0).toString(), QString("success"));
    QCOMPARE(eventSpy.at(5).at(1).toList().at(0).toString(), QString("success"));

    delete board;
}

// ── Yolo module forwards board events ───────────────────────────────────────

void TestYoloEventLifecycle::testYoloModuleForwardsEvents()
{
    Yolo yolo;
    YoloBoard* board = makeBoard("module-forward");
    blockchain_->nextInscriptionId = "insc_mod_001";

    yolo.watchBoard(board);

    QSignalSpy yoloSpy(&yolo, &Yolo::eventResponse);

    board->createPost("Module Test", "small content", "eve");

    // 4 events for small post, all forwarded through Yolo
    QCOMPARE(yoloSpy.count(), 4);
    QCOMPARE(yoloSpy.at(0).at(0).toString(), QString("post.created"));
    QCOMPARE(yoloSpy.at(3).at(0).toString(), QString("post.published"));

    delete board;
}

// ── YoloPlugin forwards events from Yolo ────────────────────────────────────

void TestYoloEventLifecycle::testYoloPluginForwardsEvents()
{
    // YoloPlugin already connects Yolo::eventResponse → YoloPlugin::eventResponse
    // in its constructor. Here we test the Yolo→Board chain which feeds that.
    Yolo yolo;
    YoloBoard* board = makeBoard("plugin-chain");
    storage_->nextCid = "QmPluginCid";
    blockchain_->nextInscriptionId = "insc_plug_001";

    yolo.watchBoard(board);

    QSignalSpy yoloSpy(&yolo, &Yolo::eventResponse);

    QString largeContent(2048, 'w');
    board->createPost("Plugin Chain", largeContent, "frank");

    // All 6 events flow through
    QCOMPARE(yoloSpy.count(), 6);

    // Verify data integrity through the chain
    QVariantList uploadedArgs = yoloSpy.at(2).at(1).toList();
    QVERIFY(uploadedArgs.at(1).toString().contains("QmPluginCid"));

    QVariantList inscribedArgs = yoloSpy.at(4).at(1).toList();
    QVERIFY(inscribedArgs.at(1).toString().contains("insc_plug_001"));

    delete board;
}

// ── Error: no lifecycle events after failure ────────────────────────────────

void TestYoloEventLifecycle::testErrorCondition_noEventAfterFailure()
{
    // Board with no blockchain → channel not available
    YoloBoard board;
    board.setKvClient(kvStub_);
    board.joinBoard("fail-board");

    QSignalSpy eventSpy(&board, &YoloBoard::eventResponse);
    QSignalSpy errorSpy(&board, &YoloBoard::error);

    board.createPost("Fail", "content", "grace");

    // No lifecycle events should fire (channel not available stops before first event)
    QCOMPARE(eventSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
}

QTEST_MAIN(TestYoloEventLifecycle)
#include "test_yolo_event_lifecycle.moc"
