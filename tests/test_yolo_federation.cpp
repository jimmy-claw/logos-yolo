#include <QtTest/QTest>
#include <QSignalSpy>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

#include "../src/yolo_board.h"
#include <federated_channel.h>
#include <channel_indexer.h>
#include <sync_types.h>
#include "logos_api_client.h"

// ── Mock blockchain that tracks per-channel inscriptions ────────────────────

class FedMockBlockchain : public LogosAPIClient {
public:
    int inscriptionCounter = 0;

    // channelId → list of inscriptions (JSON array items)
    QHash<QString, QJsonArray> channelData;

    QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                const QString& method,
                                const QVariant& arg1 = {},
                                const QVariant& arg2 = {},
                                const QVariant& /*arg3*/ = {}) override
    {
        if (method == "inscribe") {
            QString channelId = arg1.toString();
            QString dataHex = arg2.toString();

            QString inscId = QString("fed_insc_%1").arg(++inscriptionCounter);

            QJsonObject insc;
            insc["inscriptionId"] = inscId;
            insc["channelId"] = channelId;
            insc["data"] = dataHex;
            insc["slot"] = QJsonValue(static_cast<qint64>(inscriptionCounter));

            channelData[channelId].append(insc);
            return inscId;
        }
        if (method == "getChannelInscriptions") {
            QString channelId = arg1.toString();
            QJsonArray arr = channelData.value(channelId);
            return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
        }
        if (method == "queryChannelsByPrefix") {
            return QStringLiteral("[]");
        }
        return {};
    }
};

// ── Test class ──────────────────────────────────────────────────────────────

class TestYoloFederation : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testTwoAdminsPostToSameBoard();
    void testReaderSeesMergedChronological();
    void testAdminAddedSignal();
    void testAdminRemovedSignal();
    void testFederatedChannelAdminManagement();
    void testFederatedChannelFollowState();

private:
    FedMockBlockchain* blockchain_ = nullptr;
    LogosAPIClient* kvStub_ = nullptr;

    // Derive admin channel ID matching FederatedChannel's internal derivation
    QString adminChannelId(const QString& prefix, const QString& pubkey) const;
};

void TestYoloFederation::initTestCase()
{
    blockchain_ = new FedMockBlockchain();
    kvStub_ = new LogosAPIClient();
}

void TestYoloFederation::cleanupTestCase()
{
    delete blockchain_;
    delete kvStub_;
}

QString TestYoloFederation::adminChannelId(const QString& prefix,
                                            const QString& pubkey) const
{
    QString fullId = QString::fromUtf8("λ%1:%2").arg(prefix, pubkey);
    QByteArray hash = QCryptographicHash::hash(fullId.toUtf8(),
                                                QCryptographicHash::Sha256);
    return hash.toHex();
}

// ── Multi-admin tests ───────────────────────────────────────────────────────

void TestYoloFederation::testTwoAdminsPostToSameBoard()
{
    // Two FederatedChannels sharing the same prefix, each with a different admin
    QString prefix = "YOLO:shared-board";
    QString admin1 = "admin1_pubkey_aaa";
    QString admin2 = "admin2_pubkey_bbb";

    blockchain_->inscriptionCounter = 0;
    blockchain_->channelData.clear();

    // Admin 1 creates the channel and posts
    FederatedChannel chan1(prefix);
    chan1.setBlockchainClient(blockchain_);
    chan1.setKvClient(kvStub_);
    chan1.addAdmin(admin1);
    chan1.addAdmin(admin2);  // knows about admin2

    // FederatedChannel uses m_ownPubkey for inscribe - we set it indirectly
    // Since FederatedChannel doesn't expose setOwnPubkey publicly,
    // we test at the YoloBoard level instead for the full flow

    // Admin 1 inscribes directly via channel
    // The channel needs m_ownPubkey set, which happens internally
    // For direct FederatedChannel test, we verify admin management + history

    // Verify both admins get distinct channel IDs
    QString chan1Id = adminChannelId(prefix, admin1);
    QString chan2Id = adminChannelId(prefix, admin2);
    QVERIFY(chan1Id != chan2Id);
    QCOMPARE(chan1Id.length(), 64);
    QCOMPARE(chan2Id.length(), 64);

    // Simulate inscriptions from both admins directly into the mock
    QJsonObject post1;
    post1["title"] = "Admin1 Post";
    post1["author"] = admin1;
    post1["timestamp"] = qint64(1700000001000);
    QByteArray data1 = QJsonDocument(post1).toJson(QJsonDocument::Compact);

    QJsonObject insc1;
    insc1["inscriptionId"] = "fed_1";
    insc1["channelId"] = chan1Id;
    insc1["data"] = QString::fromLatin1(data1.toHex());
    insc1["slot"] = QJsonValue(qint64(1));
    blockchain_->channelData[chan1Id].append(insc1);

    QJsonObject post2;
    post2["title"] = "Admin2 Post";
    post2["author"] = admin2;
    post2["timestamp"] = qint64(1700000002000);
    QByteArray data2 = QJsonDocument(post2).toJson(QJsonDocument::Compact);

    QJsonObject insc2;
    insc2["inscriptionId"] = "fed_2";
    insc2["channelId"] = chan2Id;
    insc2["data"] = QString::fromLatin1(data2.toHex());
    insc2["slot"] = QJsonValue(qint64(2));
    blockchain_->channelData[chan2Id].append(insc2);

    // Reader follows the federated channel and gets merged history
    FederatedChannel reader(prefix);
    reader.setBlockchainClient(blockchain_);
    reader.setKvClient(kvStub_);
    reader.addAdmin(admin1);
    reader.addAdmin(admin2);
    reader.follow();

    IndexerPage page = reader.history();

    // Should see both posts, merged and sorted by slot
    QCOMPARE(page.inscriptions.size(), 2);
    QCOMPARE(page.inscriptions[0].inscriptionId, QString("fed_1"));
    QCOMPARE(page.inscriptions[1].inscriptionId, QString("fed_2"));
}

void TestYoloFederation::testReaderSeesMergedChronological()
{
    QString prefix = "YOLO:chrono-test";
    QString admin1 = "chrono_admin1";
    QString admin2 = "chrono_admin2";

    blockchain_->channelData.clear();

    QString chan1Id = adminChannelId(prefix, admin1);
    QString chan2Id = adminChannelId(prefix, admin2);

    // Interleave posts: admin2 at slot 1, admin1 at slot 2, admin2 at slot 3
    auto addInsc = [&](const QString& chanId, const QString& id,
                       const QString& title, qint64 slot) {
        QJsonObject post;
        post["title"] = title;
        QByteArray data = QJsonDocument(post).toJson(QJsonDocument::Compact);

        QJsonObject insc;
        insc["inscriptionId"] = id;
        insc["channelId"] = chanId;
        insc["data"] = QString::fromLatin1(data.toHex());
        insc["slot"] = QJsonValue(slot);
        blockchain_->channelData[chanId].append(insc);
    };

    addInsc(chan2Id, "c_1", "Admin2 First",  1);
    addInsc(chan1Id, "c_2", "Admin1 Second", 2);
    addInsc(chan2Id, "c_3", "Admin2 Third",  3);

    FederatedChannel reader(prefix);
    reader.setBlockchainClient(blockchain_);
    reader.setKvClient(kvStub_);
    reader.addAdmin(admin1);
    reader.addAdmin(admin2);
    reader.follow();

    IndexerPage page = reader.history();

    QCOMPARE(page.inscriptions.size(), 3);

    // Chronological by slot
    QCOMPARE(page.inscriptions[0].inscriptionId, QString("c_1"));
    QCOMPARE(page.inscriptions[1].inscriptionId, QString("c_2"));
    QCOMPARE(page.inscriptions[2].inscriptionId, QString("c_3"));

    // Verify data round-trip
    QJsonDocument doc = QJsonDocument::fromJson(page.inscriptions[0].data);
    QCOMPARE(doc.object()["title"].toString(), QString("Admin2 First"));
}

void TestYoloFederation::testAdminAddedSignal()
{
    FederatedChannel channel("YOLO:admin-add-test");
    channel.setBlockchainClient(blockchain_);
    channel.setKvClient(kvStub_);

    QSignalSpy addedSpy(&channel, &FederatedChannel::adminAdded);

    channel.addAdmin("new_admin_1");
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.at(0).at(0).toString(), QString("new_admin_1"));

    channel.addAdmin("new_admin_2");
    QCOMPARE(addedSpy.count(), 2);
    QCOMPARE(addedSpy.at(1).at(0).toString(), QString("new_admin_2"));

    // Duplicate add should NOT emit
    channel.addAdmin("new_admin_1");
    QCOMPARE(addedSpy.count(), 2);
}

void TestYoloFederation::testAdminRemovedSignal()
{
    FederatedChannel channel("YOLO:admin-remove-test");
    channel.setBlockchainClient(blockchain_);
    channel.setKvClient(kvStub_);

    channel.addAdmin("removable_admin");

    QSignalSpy removedSpy(&channel, &FederatedChannel::adminRemoved);

    channel.removeAdmin("removable_admin");
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(0).toString(), QString("removable_admin"));

    // Removing non-existent should NOT emit
    channel.removeAdmin("nonexistent");
    QCOMPARE(removedSpy.count(), 1);
}

void TestYoloFederation::testFederatedChannelAdminManagement()
{
    FederatedChannel channel("YOLO:mgmt-test");
    channel.setBlockchainClient(blockchain_);
    channel.setKvClient(kvStub_);

    QVERIFY(channel.admins().isEmpty());
    QVERIFY(!channel.isAdmin("admin_a"));

    channel.addAdmin("admin_a");
    channel.addAdmin("admin_b");

    QCOMPARE(channel.admins().size(), 2);
    QVERIFY(channel.isAdmin("admin_a"));
    QVERIFY(channel.isAdmin("admin_b"));
    QVERIFY(!channel.isAdmin("admin_c"));

    channel.removeAdmin("admin_a");
    QCOMPARE(channel.admins().size(), 1);
    QVERIFY(!channel.isAdmin("admin_a"));
    QVERIFY(channel.isAdmin("admin_b"));
}

void TestYoloFederation::testFederatedChannelFollowState()
{
    FederatedChannel channel("YOLO:follow-test");
    channel.setBlockchainClient(blockchain_);
    channel.setKvClient(kvStub_);

    QVERIFY(!channel.isFollowing());

    channel.addAdmin("some_admin");
    channel.follow();
    QVERIFY(channel.isFollowing());

    // Double follow is safe
    channel.follow();
    QVERIFY(channel.isFollowing());

    channel.unfollow();
    QVERIFY(!channel.isFollowing());

    // Double unfollow is safe
    channel.unfollow();
    QVERIFY(!channel.isFollowing());
}

QTEST_MAIN(TestYoloFederation)
#include "test_yolo_federation.moc"
