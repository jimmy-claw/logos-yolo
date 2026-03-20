#include "yolo_ui_component.h"
#include "yolo_plugin.h"
#include <QQuickWidget>
#include <QQmlContext>

QWidget* YoloUIComponent::createWidget(LogosAPI* logosAPI) {
    auto* quickWidget = new QQuickWidget();
    quickWidget->setMinimumSize(800, 600);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Create the SAME plugin class as the headless module (Scala pattern)
    auto* plugin = new YoloPlugin();
    plugin->setParent(quickWidget);
    plugin->initLogos(logosAPI);  // logos-app passes LogosAPI* here

    quickWidget->rootContext()->setContextProperty("yoloModule", plugin);
    quickWidget->setSource(QUrl("qrc:/yolo/main.qml"));
    return quickWidget;
}

void YoloUIComponent::destroyWidget(QWidget* widget) { delete widget; }
