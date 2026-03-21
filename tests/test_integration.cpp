#include <QtTest/QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "../src/yolo_board.h"
#include "../src/yolo.h"
#include "logos_api_client.h"

// ── Mock Clients ─────────────────────────────────────────────────────────────

class IntegMockBlockchain : public LogosAPIClient {
public:
    int inscribeCount = 0;
    QStringList inscriptionIds;
    QStringList queryResults;

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& arg1 = {},
                                const QVariant& /*arg2*/ = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "inscribe") {
            QString id = inscribeCount < inscriptionIds.size()
                ? inscriptionIds[inscribeCount]
                : QString("integ_insc_%1").arg(inscribeCount);
            inscribeCount++;
            return id;
        }
        if (method == "queryChannelsByPrefix") {
            QString prefix = arg1.toString();
            // Return discoverable channels for YOLO prefix
            QJsonArray arr;
            for (const QString& r : queryResults) {
                QJsonObject obj;
                obj["channelId"] = r;
                obj["inscriptionCount"] = 1;
                arr.append(obj);
            }
            return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
        }
        if (method == "getHistory" || method == "getInscriptions") {
            return QStringLiteral("[]");
        }
        return {};
    }
};

class IntegMockStorage : public LogosAPIClient {
public:
    QHash<QString, QByteArray> stored;
    int uploadCount = 0;

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& arg1 = {},
                                const QVariant& /*arg2*/ = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "uploadUrl") {
            QString cid = QString("QmInteg%1").arg(uploadCount++);
            stored[cid] = arg1.toByteArray();
            return cid;
        }
        if (method == "downloadUrl") {
            return stored.value(arg1.toString());
        }
        return {};
    }
};

// ── Integration Test: Full Flow ──────────────────────────────────────────────

class TestIntegration : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Full flow: create board → create post → verify events → verify post
    void testFullFlow_createBoardPostAndVerify();
    // Board discovery
    void testDiscoverBoards();
    // Multiple boards
    void testMultipleBoards();
    // Large content offload to storage
    void testLargeContentStorageOffload();
    // Yolo QML-facing API full flow
    void testYoloQmlApiFlow();

private:
    IntegMockBlockchain* blockchain_ = nullptr;
    IntegMockStorage* storage_ = nullptr;
    LogosAPIClient* kv_ = nullptr;
};

void TestIntegration::initTestCase()
{
    blockchain_ = new IntegMockBlockchain();
    storage_ = new IntegMockStorage();
    kv_ = new LogosAPIClient();
}

void TestIntegration::cleanupTestCase()
{
    delete blockchain_;
    delete storage_;
    delete kv_;
}

// ── Full flow: board → post → events → read back ────────────────────────────

void TestIntegration::testFullFlow_createBoardPostAndVerify()
{
    // Step 1: Create board
    blockchain_->inscribeCount = 0;
    blockchain_->inscriptionIds = {"board_meta_001", "post_001"};

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setStorageClient(storage_);
    board.setKvClient(kv_);
    board.setOwnPubkey("integ_test_pubkey_hex");

    QSignalSpy eventSpy(&board, &YoloBoard::eventResponse);
    QSignalSpy errorSpy(&board, &YoloBoard::error);

    board.createBoard("integration-test", "Full integration test board");

    QCOMPARE(board.boardName(), QString("integration-test"));
    QCOMPARE(board.boardPrefix(), QString("YOLO:integration-test"));

    // Step 2: Create post
    QString postId = board.createPost("Hello Integration", "This is an integration test post", "tester");

    // Should succeed (no errors)
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(!postId.isEmpty());

    // Step 3: Verify events fired (small post = 4 events)
    QCOMPARE(eventSpy.count(), 4);
    QCOMPARE(eventSpy.at(0).at(0).toString(), QString("post.created"));
    QCOMPARE(eventSpy.at(1).at(0).toString(), QString("post.inscribing"));
    QCOMPARE(eventSpy.at(2).at(0).toString(), QString("post.inscribed"));
    QCOMPARE(eventSpy.at(3).at(0).toString(), QString("post.published"));

    // Verify event types (info → info → success → success)
    QCOMPARE(eventSpy.at(0).at(1).toList().at(0).toString(), QString("info"));
    QCOMPARE(eventSpy.at(3).at(1).toList().at(0).toString(), QString("success"));

    qDebug() << "Full flow: board created, post inscribed, all 4 events fired OK";
}

// ── Board discovery ─────────────────────────────────────────────────────────

void TestIntegration::testDiscoverBoards()
{
    blockchain_->inscribeCount = 0;
    blockchain_->inscriptionIds = {"board_meta_disc"};
    blockchain_->queryResults = {"YOLO:board-alpha", "YOLO:board-beta"};

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setStorageClient(storage_);
    board.setKvClient(kv_);
    board.setOwnPubkey("disc_pubkey");

    QString json = board.discoverBoards();
    QVERIFY(!json.isEmpty());

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QVERIFY(doc.isArray());
    QCOMPARE(doc.array().size(), 2);

    // Verify board names
    QJsonObject b0 = doc.array().at(0).toObject();
    QCOMPARE(b0["channelId"].toString(), QString("YOLO:board-alpha"));

    qDebug() << "Board discovery: found" << doc.array().size() << "boards";
}

// ── Multiple boards ─────────────────────────────────────────────────────────

void TestIntegration::testMultipleBoards()
{
    blockchain_->inscribeCount = 0;
    blockchain_->inscriptionIds = {"meta_b1", "post_b1", "meta_b2", "post_b2"};

    // Board 1
    YoloBoard board1;
    board1.setBlockchainClient(blockchain_);
    board1.setStorageClient(storage_);
    board1.setKvClient(kv_);
    board1.setOwnPubkey("multi_pubkey");
    board1.createBoard("board-one", "First board");
    board1.createPost("Post on B1", "Content B1", "alice");

    // Board 2
    YoloBoard board2;
    board2.setBlockchainClient(blockchain_);
    board2.setStorageClient(storage_);
    board2.setKvClient(kv_);
    board2.setOwnPubkey("multi_pubkey");
    board2.createBoard("board-two", "Second board");
    board2.createPost("Post on B2", "Content B2", "bob");

    QCOMPARE(board1.boardPrefix(), QString("YOLO:board-one"));
    QCOMPARE(board2.boardPrefix(), QString("YOLO:board-two"));

    qDebug() << "Multiple boards: both created and posted independently";
}

// ── Large content → ContentStore offload ────────────────────────────────────

void TestIntegration::testLargeContentStorageOffload()
{
    blockchain_->inscribeCount = 0;
    blockchain_->inscriptionIds = {"meta_large", "post_large"};
    storage_->uploadCount = 0;

    YoloBoard board;
    board.setBlockchainClient(blockchain_);
    board.setStorageClient(storage_);
    board.setKvClient(kv_);
    board.setOwnPubkey("large_pubkey");
    board.createBoard("large-board", "Large content test");

    QSignalSpy eventSpy(&board, &YoloBoard::eventResponse);

    // Create a post with content > 1KB (threshold for storage offload)
    QString largeContent(2048, 'X');
    board.createPost("Large Post", largeContent, "charlie");

    // Large post should fire 6 events (including upload/uploaded)
    QCOMPARE(eventSpy.count(), 6);
    QCOMPARE(eventSpy.at(1).at(0).toString(), QString("post.uploading"));
    QCOMPARE(eventSpy.at(2).at(0).toString(), QString("post.uploaded"));

    // Verify storage was called
    QVERIFY(storage_->uploadCount > 0);

    qDebug() << "Large content: offloaded to storage, 6 events fired";
}

// ── Yolo QML API full flow (as Basecamp would use it) ───────────────────────
// Note: Yolo internally creates YoloBoard without mock clients, so posting
// won't produce an inscription. We verify the QML API layer correctly
// handles board selection, error propagation, and signal wiring.

void TestIntegration::testYoloQmlApiFlow()
{
    Yolo yolo;
    QSignalSpy boardsSpy(&yolo, &Yolo::boardsChanged);
    QSignalSpy boardChangedSpy(&yolo, &Yolo::currentBoardChanged);

    // Step 1: Initially no board selected
    QVERIFY(!yolo.hasBoardSelected());
    QVERIFY(yolo.currentBoard().isEmpty());

    // Step 2: Create a new board via QML API
    yolo.createNewBoard("qml-board", "QML integration test");
    QCOMPARE(yolo.currentBoard(), QString("qml-board"));
    QVERIFY(yolo.hasBoardSelected());
    QVERIFY(boardsSpy.count() > 0);
    QVERIFY(boardChangedSpy.count() > 0);

    // Step 3: Submit a post — will fail gracefully since no blockchain client
    // but the QML API should handle it without crashing
    yolo.submitPost("QML Post", "Posted from QML layer");

    // Step 4: Select a different board
    yolo.selectBoard("another-board");
    QCOMPARE(yolo.currentBoard(), QString("another-board"));

    // Step 5: Clear error
    yolo.clearError();
    QCOMPARE(yolo.errorMessage(), QString());

    // Step 6: hello() always works
    QVERIFY(yolo.hello().contains("Hello"));

    qDebug() << "QML API flow: board selection, error handling, signal wiring all work";
}

QTEST_MAIN(TestIntegration)
#include "test_integration.moc"
