#pragma once
#include <interface.h>
#include <QString>
#define YoloInterface_iid "org.logos.YoloInterface/1.0"
class YoloInterface : public PluginInterface {
public:
    virtual ~YoloInterface() = default;
    Q_INVOKABLE virtual QString createBoard(const QString &name, const QString &creatorPubkey) = 0;
    Q_INVOKABLE virtual QString postOpinion(const QString &prefix, const QString &authorPubkey, const QString &title, const QString &content) = 0;
    Q_INVOKABLE virtual QString getPosts(const QString &prefix) = 0;
    Q_INVOKABLE virtual void followBoard(const QString &prefix) = 0;
};
Q_DECLARE_INTERFACE(YoloInterface, YoloInterface_iid)
