#pragma once
#include <IComponent.h>
#include <QObject>
#include <QtPlugin>

class YoloUIComponent : public QObject, public IComponent {
    Q_OBJECT
    Q_INTERFACES(IComponent)
    Q_PLUGIN_METADATA(IID "com.logos.component.IComponent" FILE YOLO_UI_METADATA_FILE)

public:
    YoloUIComponent() = default;
    QWidget* createWidget(LogosAPI* logosAPI = nullptr) override;
    void destroyWidget(QWidget* widget) override;
};
