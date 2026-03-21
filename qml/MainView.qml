import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 800
    height: 600

    // The C++ backend is available as "yolo" (set via setContextProperty).

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16

        Text {
            text: "YOLO"
            font.pixelSize: 24
            font.bold: true
            color: "#212121"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: yolo ? yolo.hello() : "Loading..."
            font.pixelSize: 16
            color: "#757575"
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
