/*
 * Minimal Qt6 application that loads a QML file, renders it offscreen,
 * and saves a screenshot to a PNG file.
 *
 * Usage: ./screenshot_capture <qml_file> <output_png>
 */
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QImage>
#include <QDir>
#include <cstdio>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <qml_file> <output_png>\n", argv[0]);
        return 1;
    }

    QString qmlFile = argv[1];
    QString outputPng = argv[2];

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    engine.load(QUrl::fromLocalFile(QDir::current().absoluteFilePath(qmlFile)));

    if (engine.rootObjects().isEmpty()) {
        fprintf(stderr, "Error: failed to load %s\n", qmlFile.toUtf8().constData());
        return 1;
    }

    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (!window) {
        fprintf(stderr, "Error: root object is not a Window\n");
        return 1;
    }

    // Wait for the scene to fully render, then grab and save.
    QTimer::singleShot(1500, [&]() {
        QImage image = window->grabWindow();
        if (image.isNull()) {
            fprintf(stderr, "Error: grabWindow returned null image\n");
            QGuiApplication::exit(1);
            return;
        }

        // Ensure output directory exists.
        QDir().mkpath(QFileInfo(outputPng).absolutePath());

        if (!image.save(outputPng)) {
            fprintf(stderr, "Error: failed to save screenshot to %s\n",
                    outputPng.toUtf8().constData());
            QGuiApplication::exit(1);
            return;
        }

        printf("Screenshot saved to %s (%dx%d)\n",
               outputPng.toUtf8().constData(),
               image.width(), image.height());
        QGuiApplication::exit(0);
    });

    return app.exec();
}
