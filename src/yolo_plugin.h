#pragma once
#include <QObject>
#include "yolo_interface.h"
class YoloBoard;
class LogosAPI;

class YoloPlugin : public QObject, public YoloInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID YoloInterface_iid FILE "metadata.json")
    Q_INTERFACES(YoloInterface PluginInterface)
public:
    explicit YoloPlugin(QObject *parent = nullptr);
    QString name() const override { return QStringLiteral("yolo"); }
    Q_INVOKABLE QString version() const override { return QStringLiteral("0.1.0"); }
    Q_INVOKABLE void initLogos(LogosAPI *api);
    Q_INVOKABLE QString createBoard(const QString &name, const QString &creatorPubkey) override;
    Q_INVOKABLE QString postOpinion(const QString &prefix, const QString &authorPubkey, const QString &title, const QString &content) override;
    Q_INVOKABLE QString getPosts(const QString &prefix) override;
    Q_INVOKABLE void followBoard(const QString &prefix) override;
signals:
    void eventResponse(const QString &eventName, const QVariantList &args);
private:
    YoloBoard *m_board = nullptr;
    LogosAPI *m_logosAPI = nullptr;
};

