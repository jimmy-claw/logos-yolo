#include "yolo_ui_component.h"
#include "yolo.h"

#include <QQuickWidget>
#include <QQmlContext>

QWidget* YoloUIComponent::createWidget(LogosAPI* logosAPI) {
    auto* quickWidget = new QQuickWidget();
    quickWidget->setMinimumSize(800, 600);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Create the C++ backend and parent it to the widget (auto-cleanup)
    auto* backend = new Yolo();
    backend->setParent(quickWidget);

#ifdef LOGOS_CORE_AVAILABLE
    if (logosAPI) {
        backend->initLogos(logosAPI);
    }
#endif

    // Expose the backend to QML as "yolo"
    quickWidget->rootContext()->setContextProperty("yolo", backend);

    // Load the main QML file from the embedded resource
    quickWidget->setSource(QUrl("qrc:/yolo/MainView.qml"));

    return quickWidget;
}

void YoloUIComponent::destroyWidget(QWidget* widget) {
    delete widget;
}
