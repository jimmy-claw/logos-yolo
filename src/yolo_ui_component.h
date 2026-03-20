#pragma once
#include <IComponent.h>
#include <QObject>

#define YOLO_IID "com.logos.component.IComponent"

class YoloUIComponent : public QObject, public IComponent {
    Q_OBJECT
    Q_INTERFACES(IComponent)
    Q_PLUGIN_METADATA(IID YOLO_IID FILE YOLO_UI_METADATA_FILE)

public:
    QWidget* createWidget(LogosAPI* logosAPI = nullptr) override;
    void destroyWidget(QWidget* widget) override;
};
