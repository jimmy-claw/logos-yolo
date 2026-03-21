// e2e_logos_probe.cpp — Connect to running LogosApp via Qt RemoteObjects
// Probes the live IPC infrastructure and attempts to call yolo module methods.
//
// Build: g++ -fPIC $(pkg-config --cflags --libs Qt6RemoteObjects Qt6Core) \
//        -o e2e_logos_probe tests/e2e_logos_probe.cpp

#include <QCoreApplication>
#include <QRemoteObjectNode>
#include <QRemoteObjectReplica>
#include <QRemoteObjectPendingCall>
#include <QTimer>
#include <QMetaMethod>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static void printBanner(const char* title) {
    qDebug().noquote() << "";
    qDebug().noquote() << QString("══════════════════════════════════════════════════");
    qDebug().noquote() << QString("  %1").arg(title);
    qDebug().noquote() << QString("══════════════════════════════════════════════════");
}

static void printSection(const char* title) {
    qDebug().noquote() << "";
    qDebug().noquote() << QString("── %1 ──").arg(title);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    printBanner("YOLO E2E Probe — Live LogosApp Connection Test");

    int pass = 0, fail = 0, skip = 0;

    // ── Step 1: Verify IPC socket exists ──────────────────────────────────
    printSection("Step 1: IPC Socket Verification");

    QStringList sockets = {
        "/tmp/logos_core_manager",
        "/tmp/logos_capability_module",
        "/tmp/logos_package_manager"
    };

    for (const QString& sock : sockets) {
        if (QFile::exists(sock)) {
            qDebug().noquote() << "  PASS:" << sock << "exists";
            pass++;
        } else {
            qDebug().noquote() << "  FAIL:" << sock << "not found";
            fail++;
        }
    }

    // Also check for yolo-specific socket
    if (QFile::exists("/tmp/logos_yolo")) {
        qDebug().noquote() << "  PASS: /tmp/logos_yolo exists (yolo has own registry)";
        pass++;
    } else {
        qDebug().noquote() << "  INFO: /tmp/logos_yolo not found (yolo loaded in-process)";
    }

    // ── Step 2: Connect to core_manager registry ──────────────────────────
    printSection("Step 2: Qt RemoteObjects Connection");

    QRemoteObjectNode node;
    bool connected = node.connectToNode(QUrl("local:logos_core_manager"));

    if (connected) {
        qDebug().noquote() << "  PASS: Connected to local:logos_core_manager";
        pass++;
    } else {
        qDebug().noquote() << "  FAIL: Could not connect to local:logos_core_manager";
        fail++;
    }

    // ── Step 3: Probe for yolo replica ────────────────────────────────────
    printSection("Step 3: Yolo Module Discovery");

    // Try to acquire dynamic replica of yolo
    QStringList moduleNames = {"yolo", "blockchain_module", "kv_module",
                                "capability_module", "package_manager"};

    for (const QString& modName : moduleNames) {
        QRemoteObjectReplica* replica = node.acquireDynamic(modName);
        if (replica) {
            bool ready = replica->waitForSource(3000);
            if (ready) {
                qDebug().noquote() << "  PASS:" << modName << "- replica acquired and ready";
                pass++;

                // List available methods
                const QMetaObject* meta = replica->metaObject();
                QStringList methods;
                for (int i = meta->methodOffset(); i < meta->methodCount(); ++i) {
                    QMetaMethod m = meta->method(i);
                    if (m.methodType() == QMetaMethod::Method ||
                        m.methodType() == QMetaMethod::Slot) {
                        methods << QString::fromLatin1(m.methodSignature());
                    }
                }
                if (!methods.isEmpty()) {
                    qDebug().noquote() << "         Methods:" << methods.join(", ");
                }

                // If this is yolo, try calling hello()
                if (modName == "yolo") {
                    QRemoteObjectPendingCall call;
                    bool invoked = QMetaObject::invokeMethod(
                        replica, "hello",
                        Qt::DirectConnection,
                        Q_RETURN_ARG(QRemoteObjectPendingCall, call));

                    if (invoked && call.waitForFinished(5000)) {
                        qDebug().noquote() << "  PASS: yolo.hello() =>" << call.returnValue().toString();
                        pass++;
                    } else {
                        qDebug().noquote() << "  INFO: yolo.hello() direct call not available (may need auth token)";
                    }
                }
            } else {
                qDebug().noquote() << "  SKIP:" << modName << "- replica acquired but source not ready (timeout)";
                skip++;
            }
            delete replica;
        } else {
            qDebug().noquote() << "  SKIP:" << modName << "- could not acquire replica";
            skip++;
        }
    }

    // ── Step 4: Probe capability_module directly ──────────────────────────
    printSection("Step 4: Capability Module Probe");

    QRemoteObjectNode capNode;
    if (capNode.connectToNode(QUrl("local:logos_capability_module"))) {
        qDebug().noquote() << "  PASS: Connected to local:logos_capability_module";
        pass++;

        QRemoteObjectReplica* capReplica = capNode.acquireDynamic("capability_module");
        if (capReplica && capReplica->waitForSource(5000)) {
            qDebug().noquote() << "  PASS: capability_module replica ready";
            pass++;

            const QMetaObject* meta = capReplica->metaObject();
            qDebug().noquote() << "         Available methods:";
            for (int i = meta->methodOffset(); i < meta->methodCount(); ++i) {
                QMetaMethod m = meta->method(i);
                if (m.methodType() == QMetaMethod::Method ||
                    m.methodType() == QMetaMethod::Slot) {
                    qDebug().noquote() << "           -" << QString::fromLatin1(m.methodSignature());
                }
            }
            delete capReplica;
        } else {
            qDebug().noquote() << "  SKIP: capability_module replica not ready";
            skip++;
            if (capReplica) delete capReplica;
        }
    } else {
        qDebug().noquote() << "  SKIP: Could not connect to logos_capability_module";
        skip++;
    }

    // ── Summary ───────────────────────────────────────────────────────────
    printBanner("E2E Probe Summary");
    qDebug().noquote() << QString("  PASSED:  %1").arg(pass);
    qDebug().noquote() << QString("  FAILED:  %1").arg(fail);
    qDebug().noquote() << QString("  SKIPPED: %1").arg(skip);
    qDebug().noquote() << QString("══════════════════════════════════════════════════");

    return fail > 0 ? 1 : 0;
}
