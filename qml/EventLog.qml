import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "#263238"

    ListModel { id: eventModel }

    Connections {
        target: yolo
        function onNewEvent(eventName, type, message) {
            eventModel.append({
                "eventName": eventName,
                "eventType": type,
                "message": message,
                "time": Qt.formatTime(new Date(), "hh:mm:ss")
            })
            eventList.positionViewAtEnd()
            // Cap at 100 entries
            while (eventModel.count > 100) {
                eventModel.remove(0)
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: "#1B2327"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                    text: "Event Log"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#B0BEC5"
                    Layout.fillWidth: true
                }

                Label {
                    text: eventModel.count + " events"
                    font.pixelSize: 11
                    color: "#78909C"
                }

                ToolButton {
                    contentItem: Label {
                        text: "Clear"
                        color: "#78909C"
                        font.pixelSize: 11
                    }
                    background: Item {}
                    onClicked: eventModel.clear()
                }
            }
        }

        // Event list
        ListView {
            id: eventList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: eventModel
            clip: true
            spacing: 0

            delegate: Rectangle {
                width: ListView.view ? ListView.view.width : 0
                height: eventRow.implicitHeight + 4
                color: index % 2 === 0 ? "transparent" : "#1E2A30"

                RowLayout {
                    id: eventRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Label {
                        text: model.time
                        font.pixelSize: 11
                        font.family: "monospace"
                        color: "#78909C"
                    }

                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: {
                            if (model.eventType === "success") return "#43A047"
                            if (model.eventType === "error") return "#E53935"
                            return "#1565C0"
                        }
                    }

                    Label {
                        text: model.message
                        font.pixelSize: 12
                        font.family: "monospace"
                        color: {
                            if (model.eventType === "success") return "#81C784"
                            if (model.eventType === "error") return "#EF9A9A"
                            return "#B0BEC5"
                        }
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                }
            }

            // Empty state
            Label {
                anchors.centerIn: parent
                visible: eventModel.count === 0
                text: "No events yet"
                font.pixelSize: 12
                color: "#546E7A"
            }
        }
    }
}
