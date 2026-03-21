#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class LogosAPI;  // Forward declaration
class YoloBoard; // Forward declaration

/**
 * Yolo — YOLO community board logic for Logos UI plugin.
 *
 * Owns YoloBoard instances and forwards their eventResponse signals
 * so that YoloPlugin can route them to Logos Core for UI/CLI display.
 */
class Yolo : public QObject {
    Q_OBJECT

public:
    explicit Yolo(QObject *parent = nullptr);

    Q_INVOKABLE QString hello() const;
    void initLogos(LogosAPI *logosAPIInstance);

    // Connect a YoloBoard's eventResponse to this module's eventResponse.
    void watchBoard(YoloBoard *board);

signals:
    void eventResponse(const QString &eventName, const QVariantList &args);
};
