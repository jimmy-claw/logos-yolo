#pragma once

#include <IComponent.h>
#include <QObject>

/**
 * YoloUIComponent — IComponent plugin for logos-app.
 *
 * This is the entry point that logos-app loads. It creates a QQuickWidget
 * containing the QML UI and wires up the C++ backend (Yolo) as a
 * QML context property.
 */
class YoloUIComponent : public QObject, public IComponent {
    Q_OBJECT
    Q_INTERFACES(IComponent)
    Q_PLUGIN_METADATA(IID IComponent_iid FILE YOLO_UI_METADATA_FILE)

public:
    QWidget* createWidget(LogosAPI* logosAPI = nullptr) override;
    void destroyWidget(QWidget* widget) override;
};
