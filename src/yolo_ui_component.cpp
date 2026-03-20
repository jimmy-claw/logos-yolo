#include "yolo_ui_component.h"
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

QWidget* YoloUIComponent::createWidget(LogosAPI* logosAPI) {
    Q_UNUSED(logosAPI)
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);
    auto* label = new QLabel("YOLO - Your Own Local Opinion");
    layout->addWidget(label);
    return widget;
}

void YoloUIComponent::destroyWidget(QWidget* widget) {
    delete widget;
}
