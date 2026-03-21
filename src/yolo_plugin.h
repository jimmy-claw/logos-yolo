#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include "yolo.h"

#include <interface.h>
#include <logos_api.h>

/**
 * YoloPlugin — Headless logoscore plugin wrapper.
 *
 * Loaded by logos_host as a shared library. Wraps Yolo and exposes its
 * methods via QtRO (Qt Remote Objects) for other modules and the QML UI to call.
 *
 * Must NOT use Qt Quick, QML engine, or any GUI classes — only Qt Core/Qml/RemoteObjects.
 */
class YoloPlugin : public QObject, public PluginInterface {
    Q_OBJECT
#ifndef YOLO_UI_BUILD
    Q_PLUGIN_METADATA(IID PluginInterface_iid FILE "metadata.json")
#endif
    Q_INTERFACES(PluginInterface)

public:
    explicit YoloPlugin(QObject *parent = nullptr);

    // ── PluginInterface ─────────────────────────────────────────────────────
    [[nodiscard]] QString name() const override { return QStringLiteral("yolo"); }
    Q_INVOKABLE QString version() const override { return QStringLiteral("0.1.0"); }
    Q_INVOKABLE void initLogos(LogosAPI *api);

    Q_INVOKABLE QString hello() const;

signals:
    void eventResponse(const QString &eventName, const QVariantList &args);

private:
    Yolo *m_module = nullptr;
    LogosAPI *m_logosAPI = nullptr;
};
