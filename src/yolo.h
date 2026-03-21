#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class LogosAPI;  // Forward declaration

/**
 * Yolo — YOLO community board logic for Logos UI plugin.
 */
class Yolo : public QObject {
    Q_OBJECT

public:
    explicit Yolo(QObject *parent = nullptr);

    Q_INVOKABLE QString hello() const;
    void initLogos(LogosAPI *logosAPIInstance);

signals:
    void eventResponse(const QString &eventName, const QVariantList &args);
};
