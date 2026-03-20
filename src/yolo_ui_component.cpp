#include "yolo_ui_component.h"
#include <QWidget>

QWidget* YoloUIComponent::createWidget(LogosAPI* logosAPI) {
    Q_UNUSED(logosAPI)
    return new QWidget();
}

void YoloUIComponent::destroyWidget(QWidget* widget) {
    delete widget;
}
